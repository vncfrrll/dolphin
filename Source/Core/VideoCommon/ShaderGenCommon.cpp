// Copyright 2017 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include "VideoCommon/ShaderGenCommon.h"

#include <fmt/format.h>

#include "Common/FileUtil.h"
#include "Core/ConfigManager.h"
#include "VideoCommon/VideoCommon.h"
#include "VideoCommon/VideoConfig.h"

ShaderHostConfig ShaderHostConfig::GetCurrent()
{
  ShaderHostConfig bits = {};
  bits.msaa = g_ActiveConfig.iMultisamples > 1;
  bits.ssaa = g_ActiveConfig.iMultisamples > 1 && g_ActiveConfig.bSSAA &&
              g_ActiveConfig.backend_info.bSupportsSSAA;
  bits.stereo = g_ActiveConfig.stereo_mode != StereoMode::Off;
  bits.wireframe = g_ActiveConfig.bWireFrame;
  bits.per_pixel_lighting = g_ActiveConfig.bEnablePixelLighting;
  bits.vertex_rounding = g_ActiveConfig.UseVertexRounding();
  bits.fast_depth_calc = g_ActiveConfig.bFastDepthCalc;
  bits.bounding_box = g_ActiveConfig.bBBoxEnable;
  bits.backend_dual_source_blend = g_ActiveConfig.backend_info.bSupportsDualSourceBlend;
  bits.backend_geometry_shaders = g_ActiveConfig.backend_info.bSupportsGeometryShaders;
  bits.backend_early_z = g_ActiveConfig.backend_info.bSupportsEarlyZ;
  bits.backend_bbox = g_ActiveConfig.backend_info.bSupportsBBox;
  bits.backend_gs_instancing = g_ActiveConfig.backend_info.bSupportsGSInstancing;
  bits.backend_clip_control = g_ActiveConfig.backend_info.bSupportsClipControl;
  bits.backend_ssaa = g_ActiveConfig.backend_info.bSupportsSSAA;
  bits.backend_atomics = g_ActiveConfig.backend_info.bSupportsFragmentStoresAndAtomics;
  bits.backend_depth_clamp = g_ActiveConfig.backend_info.bSupportsDepthClamp;
  bits.backend_reversed_depth_range = g_ActiveConfig.backend_info.bSupportsReversedDepthRange;
  bits.backend_bitfield = g_ActiveConfig.backend_info.bSupportsBitfield;
  bits.backend_dynamic_sampler_indexing =
      g_ActiveConfig.backend_info.bSupportsDynamicSamplerIndexing;
  bits.backend_shader_framebuffer_fetch = g_ActiveConfig.backend_info.bSupportsFramebufferFetch;
  bits.backend_logic_op = g_ActiveConfig.backend_info.bSupportsLogicOp;
  bits.backend_palette_conversion = g_ActiveConfig.backend_info.bSupportsPaletteConversion;
  bits.enable_validation_layer = g_ActiveConfig.bEnableValidationLayer;
  bits.manual_texture_sampling = !g_ActiveConfig.bFastTextureSampling;
  bits.manual_texture_sampling_custom_texture_sizes =
      g_ActiveConfig.ManualTextureSamplingWithHiResTextures();
  bits.backend_sampler_lod_bias = g_ActiveConfig.backend_info.bSupportsLodBiasInSampler;
  return bits;
}

std::string GetDiskShaderCacheFileName(APIType api_type, const char* type, bool include_gameid,
                                       bool include_host_config, bool include_api)
{
  if (!File::Exists(File::GetUserPath(D_SHADERCACHE_IDX)))
    File::CreateDir(File::GetUserPath(D_SHADERCACHE_IDX));

  std::string filename = File::GetUserPath(D_SHADERCACHE_IDX);
  if (include_api)
  {
    switch (api_type)
    {
    case APIType::D3D:
      filename += "D3D";
      break;
    case APIType::Metal:
      filename += "Metal";
      break;
    case APIType::OpenGL:
      filename += "OpenGL";
      break;
    case APIType::Vulkan:
      filename += "Vulkan";
      break;
    default:
      break;
    }
    filename += '-';
  }

  filename += type;

  if (include_gameid)
  {
    filename += '-';
    filename += SConfig::GetInstance().GetGameID();
  }

  if (include_host_config)
  {
    // We're using 21 bits, so 6 hex characters.
    ShaderHostConfig host_config = ShaderHostConfig::GetCurrent();
    filename += fmt::format("-{:06X}", host_config.bits);
  }

  filename += ".cache";
  return filename;
}

void WriteIsNanHeader(ShaderCode& out, APIType api_type)
{
  out.Write("#define dolphin_isnan(f) isnan(f)\n");
}

void WriteBitfieldExtractHeader(ShaderCode& out, APIType api_type,
                                const ShaderHostConfig& host_config)
{
  // ==============================================
  //  BitfieldExtract for APIs which don't have it
  // ==============================================
  if (!host_config.backend_bitfield)
  {
    out.Write("uint bitfieldExtract(uint val, int off, int size) {{\n"
              "  // This built-in function is only supported in OpenGL 4.0+ and ES 3.1+\n"
              "  // Microsoft's HLSL compiler automatically optimises this to a bitfield extract "
              "instruction.\n"
              "  uint mask = uint((1 << size) - 1);\n"
              "  return uint(val >> off) & mask;\n"
              "}}\n\n");
    out.Write("int bitfieldExtract(int val, int off, int size) {{\n"
              "  // This built-in function is only supported in OpenGL 4.0+ and ES 3.1+\n"
              "  // Microsoft's HLSL compiler automatically optimises this to a bitfield extract "
              "instruction.\n"
              "  return ((val << (32 - size - off)) >> (32 - size));\n"
              "}}\n\n");
  }
}

static void DefineOutputMember(ShaderCode& object, APIType api_type, std::string_view qualifier,
                               std::string_view type, std::string_view name, int var_index,
                               ShaderStage stage, std::string_view semantic = {},
                               int semantic_index = -1)
{
  object.Write("\t{} {} {}", qualifier, type, name);

  if (var_index != -1)
    object.Write("{}", var_index);

  if (api_type == APIType::D3D && !semantic.empty() && stage == ShaderStage::Geometry)
  {
    if (semantic_index != -1)
      object.Write(" : {}{}", semantic, semantic_index);
    else
      object.Write(" : {}", semantic);
  }

  object.Write(";\n");
}

void GenerateVSOutputMembers(ShaderCode& object, APIType api_type, u32 texgens,
                             const ShaderHostConfig& host_config, std::string_view qualifier,
                             ShaderStage stage)
{
  // SPIRV-Cross names all semantics as "TEXCOORD"
  // Unfortunately Geometry shaders (which also uses this function)
  // aren't supported.  The output semantic name needs to match
  // up with the input semantic name for both the next stage (pixel shader)
  // and the previous stage (vertex shader), so
  // we need to handle geometry in a special way...
  if (api_type == APIType::D3D && stage == ShaderStage::Geometry)
  {
    DefineOutputMember(object, api_type, qualifier, "float4", "pos", -1, stage, "TEXCOORD", 0);
    DefineOutputMember(object, api_type, qualifier, "float4", "colors_", 0, stage, "TEXCOORD", 1);
    DefineOutputMember(object, api_type, qualifier, "float4", "colors_", 1, stage, "TEXCOORD", 2);

    const unsigned int index_base = 3;
    unsigned int index_offset = 0;
    if (host_config.backend_geometry_shaders)
    {
      DefineOutputMember(object, api_type, qualifier, "float", "clipDist", 0, stage, "TEXCOORD",
                         index_base + index_offset);
      DefineOutputMember(object, api_type, qualifier, "float", "clipDist", 1, stage, "TEXCOORD",
                         index_base + index_offset + 1);
      index_offset += 2;
    }

    for (unsigned int i = 0; i < texgens; ++i)
    {
      DefineOutputMember(object, api_type, qualifier, "float3", "tex", i, stage, "TEXCOORD",
                         index_base + index_offset + i);
    }
    index_offset += texgens;

    if (!host_config.fast_depth_calc)
    {
      DefineOutputMember(object, api_type, qualifier, "float4", "clipPos", -1, stage, "TEXCOORD",
                         index_base + index_offset);
      index_offset++;
    }

    if (host_config.per_pixel_lighting)
    {
      DefineOutputMember(object, api_type, qualifier, "float3", "Normal", -1, stage, "TEXCOORD",
                         index_base + index_offset);
      DefineOutputMember(object, api_type, qualifier, "float3", "WorldPos", -1, stage, "TEXCOORD",
                         index_base + index_offset + 1);
      index_offset += 2;
    }
  }
  else
  {
    DefineOutputMember(object, api_type, qualifier, "float4", "pos", -1, stage, "SV_Position");
    DefineOutputMember(object, api_type, qualifier, "float4", "colors_", 0, stage, "COLOR", 0);
    DefineOutputMember(object, api_type, qualifier, "float4", "colors_", 1, stage, "COLOR", 1);

    if (host_config.backend_geometry_shaders)
    {
      DefineOutputMember(object, api_type, qualifier, "float", "clipDist", 0, stage,
                         "SV_ClipDistance", 0);
      DefineOutputMember(object, api_type, qualifier, "float", "clipDist", 1, stage,
                         "SV_ClipDistance", 1);
    }

    for (unsigned int i = 0; i < texgens; ++i)
      DefineOutputMember(object, api_type, qualifier, "float3", "tex", i, stage, "TEXCOORD", i);

    if (!host_config.fast_depth_calc)
      DefineOutputMember(object, api_type, qualifier, "float4", "clipPos", -1, stage, "TEXCOORD",
                         texgens);

    if (host_config.per_pixel_lighting)
    {
      DefineOutputMember(object, api_type, qualifier, "float3", "Normal", -1, stage, "TEXCOORD",
                         texgens + 1);
      DefineOutputMember(object, api_type, qualifier, "float3", "WorldPos", -1, stage, "TEXCOORD",
                         texgens + 2);
    }
  }
}

void AssignVSOutputMembers(ShaderCode& object, std::string_view a, std::string_view b, u32 texgens,
                           const ShaderHostConfig& host_config)
{
  object.Write("\t{}.pos = {}.pos;\n", a, b);
  object.Write("\t{}.colors_0 = {}.colors_0;\n", a, b);
  object.Write("\t{}.colors_1 = {}.colors_1;\n", a, b);

  for (unsigned int i = 0; i < texgens; ++i)
    object.Write("\t{}.tex{} = {}.tex{};\n", a, i, b, i);

  if (!host_config.fast_depth_calc)
    object.Write("\t{}.clipPos = {}.clipPos;\n", a, b);

  if (host_config.per_pixel_lighting)
  {
    object.Write("\t{}.Normal = {}.Normal;\n", a, b);
    object.Write("\t{}.WorldPos = {}.WorldPos;\n", a, b);
  }

  if (host_config.backend_geometry_shaders)
  {
    object.Write("\t{}.clipDist0 = {}.clipDist0;\n", a, b);
    object.Write("\t{}.clipDist1 = {}.clipDist1;\n", a, b);
  }
}

const char* GetInterpolationQualifier(bool msaa, bool ssaa, bool in_glsl_interface_block, bool in)
{
  if (!msaa)
    return "";

  // Without GL_ARB_shading_language_420pack support, the interpolation qualifier must be
  // "centroid in" and not "centroid", even within an interface block.
  if (in_glsl_interface_block && !g_ActiveConfig.backend_info.bSupportsBindingLayout)
  {
    if (!ssaa)
      return in ? "centroid in" : "centroid out";
    else
      return in ? "sample in" : "sample out";
  }
  else
  {
    if (!ssaa)
      return "centroid";
    else
      return "sample";
  }
}
