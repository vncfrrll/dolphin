// Copyright 2018 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include "jni/AndroidCommon/IDCache.h"

#include <jni.h>

static constexpr jint JNI_VERSION = JNI_VERSION_1_6;

static JavaVM* s_java_vm;

static jclass s_string_class;

static jclass s_native_library_class;
static jmethodID s_display_alert_msg;
static jmethodID s_do_rumble;
static jmethodID s_update_touch_pointer;
static jmethodID s_on_title_changed;
static jmethodID s_finish_emulation_activity;

static jclass s_game_file_class;
static jfieldID s_game_file_pointer;
static jmethodID s_game_file_constructor;

static jclass s_game_file_cache_class;
static jfieldID s_game_file_cache_pointer;

static jclass s_analytics_class;
static jmethodID s_send_analytics_report;
static jmethodID s_get_analytics_value;

static jclass s_linked_hash_map_class;
static jmethodID s_linked_hash_map_init;
static jmethodID s_linked_hash_map_put;

static jclass s_ini_file_class;
static jfieldID s_ini_file_pointer;
static jclass s_ini_file_section_class;
static jfieldID s_ini_file_section_pointer;
static jmethodID s_ini_file_section_constructor;

static jclass s_compress_cb_class;
static jmethodID s_compress_cb_run;

static jclass s_content_handler_class;
static jmethodID s_content_handler_open_fd;
static jmethodID s_content_handler_delete;
static jmethodID s_content_handler_get_size_and_is_directory;
static jmethodID s_content_handler_get_display_name;
static jmethodID s_content_handler_get_child_names;
static jmethodID s_content_handler_do_file_search;

static jclass s_network_helper_class;
static jmethodID s_network_helper_get_network_ip_address;
static jmethodID s_network_helper_get_network_prefix_length;
static jmethodID s_network_helper_get_network_gateway;

static jclass s_boolean_supplier_class;
static jmethodID s_boolean_supplier_get;

static jclass s_ar_cheat_class;
static jfieldID s_ar_cheat_pointer;
static jmethodID s_ar_cheat_constructor;

static jclass s_gecko_cheat_class;
static jfieldID s_gecko_cheat_pointer;
static jmethodID s_gecko_cheat_constructor;

static jclass s_patch_cheat_class;
static jfieldID s_patch_cheat_pointer;
static jmethodID s_patch_cheat_constructor;

static jclass s_graphics_mod_group_class;
static jfieldID s_graphics_mod_group_pointer;
static jmethodID s_graphics_mod_group_constructor;

static jclass s_graphics_mod_class;
static jfieldID s_graphics_mod_pointer;
static jmethodID s_graphics_mod_constructor;

static jclass s_riivolution_patches_class;
static jfieldID s_riivolution_patches_pointer;

static jclass s_wii_update_cb_class;
static jmethodID s_wii_update_cb_run;

namespace IDCache
{
JNIEnv* GetEnvForThread()
{
  thread_local static struct OwnedEnv
  {
    OwnedEnv()
    {
      status = s_java_vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
      if (status == JNI_EDETACHED)
        s_java_vm->AttachCurrentThread(&env, nullptr);
    }

    ~OwnedEnv()
    {
      if (status == JNI_EDETACHED)
        s_java_vm->DetachCurrentThread();
    }

    int status;
    JNIEnv* env = nullptr;
  } owned;
  return owned.env;
}

jclass GetStringClass()
{
  return s_string_class;
}

jclass GetNativeLibraryClass()
{
  return s_native_library_class;
}

jmethodID GetDisplayAlertMsg()
{
  return s_display_alert_msg;
}

jmethodID GetDoRumble()
{
  return s_do_rumble;
}

jmethodID GetUpdateTouchPointer()
{
  return s_update_touch_pointer;
}

jmethodID GetOnTitleChanged()
{
  return s_on_title_changed;
}

jmethodID GetFinishEmulationActivity()
{
  return s_finish_emulation_activity;
}

jclass GetAnalyticsClass()
{
  return s_analytics_class;
}

jmethodID GetSendAnalyticsReport()
{
  return s_send_analytics_report;
}

jmethodID GetAnalyticsValue()
{
  return s_get_analytics_value;
}

jclass GetGameFileClass()
{
  return s_game_file_class;
}

jfieldID GetGameFilePointer()
{
  return s_game_file_pointer;
}

jmethodID GetGameFileConstructor()
{
  return s_game_file_constructor;
}

jclass GetGameFileCacheClass()
{
  return s_game_file_cache_class;
}

jfieldID GetGameFileCachePointer()
{
  return s_game_file_cache_pointer;
}

jclass GetLinkedHashMapClass()
{
  return s_linked_hash_map_class;
}

jmethodID GetLinkedHashMapInit()
{
  return s_linked_hash_map_init;
}

jmethodID GetLinkedHashMapPut()
{
  return s_linked_hash_map_put;
}

jclass GetIniFileClass()
{
  return s_ini_file_class;
}

jfieldID GetIniFilePointer()
{
  return s_ini_file_pointer;
}

jclass GetIniFileSectionClass()
{
  return s_ini_file_section_class;
}

jfieldID GetIniFileSectionPointer()
{
  return s_ini_file_section_pointer;
}

jmethodID GetIniFileSectionConstructor()
{
  return s_ini_file_section_constructor;
}

jclass GetCompressCallbackClass()
{
  return s_compress_cb_class;
}

jmethodID GetCompressCallbackRun()
{
  return s_compress_cb_run;
}

jclass GetContentHandlerClass()
{
  return s_content_handler_class;
}

jmethodID GetContentHandlerOpenFd()
{
  return s_content_handler_open_fd;
}

jmethodID GetContentHandlerDelete()
{
  return s_content_handler_delete;
}

jmethodID GetContentHandlerGetSizeAndIsDirectory()
{
  return s_content_handler_get_size_and_is_directory;
}

jmethodID GetContentHandlerGetDisplayName()
{
  return s_content_handler_get_display_name;
}

jmethodID GetContentHandlerGetChildNames()
{
  return s_content_handler_get_child_names;
}

jmethodID GetContentHandlerDoFileSearch()
{
  return s_content_handler_do_file_search;
}

jclass GetNetworkHelperClass()
{
  return s_network_helper_class;
}

jmethodID GetNetworkHelperGetNetworkIpAddress()
{
  return s_network_helper_get_network_ip_address;
}

jmethodID GetNetworkHelperGetNetworkPrefixLength()
{
  return s_network_helper_get_network_prefix_length;
}

jmethodID GetNetworkHelperGetNetworkGateway()
{
  return s_network_helper_get_network_gateway;
}

jmethodID GetBooleanSupplierGet()
{
  return s_boolean_supplier_get;
}

jclass GetARCheatClass()
{
  return s_ar_cheat_class;
}

jfieldID GetARCheatPointer()
{
  return s_ar_cheat_pointer;
}

jmethodID GetARCheatConstructor()
{
  return s_ar_cheat_constructor;
}

jclass GetGeckoCheatClass()
{
  return s_gecko_cheat_class;
}

jfieldID GetGeckoCheatPointer()
{
  return s_gecko_cheat_pointer;
}

jmethodID GetGeckoCheatConstructor()
{
  return s_gecko_cheat_constructor;
}

jclass GetPatchCheatClass()
{
  return s_patch_cheat_class;
}

jfieldID GetPatchCheatPointer()
{
  return s_patch_cheat_pointer;
}

jmethodID GetPatchCheatConstructor()
{
  return s_patch_cheat_constructor;
}

jclass GetGraphicsModClass()
{
  return s_graphics_mod_class;
}

jfieldID GetGraphicsModPointer()
{
  return s_graphics_mod_pointer;
}

jmethodID GetGraphicsModConstructor()
{
  return s_graphics_mod_constructor;
}

jclass GetGraphicsModGroupClass()
{
  return s_graphics_mod_group_class;
}

jfieldID GetGraphicsModGroupPointer()
{
  return s_graphics_mod_group_pointer;
}

jmethodID GetGraphicsModGroupConstructor()
{
  return s_graphics_mod_group_constructor;
}

jclass GetRiivolutionPatchesClass()
{
  return s_riivolution_patches_class;
}

jfieldID GetRiivolutionPatchesPointer()
{
  return s_riivolution_patches_pointer;
}

jclass GetWiiUpdateCallbackClass()
{
  return s_wii_update_cb_class;
}

jmethodID GetWiiUpdateCallbackFunction()
{
  return s_wii_update_cb_run;
}

}  // namespace IDCache

extern "C" {

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
  s_java_vm = vm;

  JNIEnv* env;
  if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION) != JNI_OK)
    return JNI_ERR;

  const jclass string_class = env->FindClass("java/lang/String");
  s_string_class = reinterpret_cast<jclass>(env->NewGlobalRef(string_class));

  const jclass native_library_class = env->FindClass("org/dolphinemu/dolphinemu/NativeLibrary");
  s_native_library_class = reinterpret_cast<jclass>(env->NewGlobalRef(native_library_class));
  s_display_alert_msg = env->GetStaticMethodID(s_native_library_class, "displayAlertMsg",
                                               "(Ljava/lang/String;Ljava/lang/String;ZZZ)Z");
  s_do_rumble = env->GetStaticMethodID(s_native_library_class, "rumble", "(ID)V");
  s_update_touch_pointer =
      env->GetStaticMethodID(s_native_library_class, "updateTouchPointer", "()V");
  s_on_title_changed = env->GetStaticMethodID(s_native_library_class, "onTitleChanged", "()V");
  s_finish_emulation_activity =
      env->GetStaticMethodID(s_native_library_class, "finishEmulationActivity", "()V");
  env->DeleteLocalRef(native_library_class);

  const jclass game_file_class = env->FindClass("org/dolphinemu/dolphinemu/model/GameFile");
  s_game_file_class = reinterpret_cast<jclass>(env->NewGlobalRef(game_file_class));
  s_game_file_pointer = env->GetFieldID(game_file_class, "mPointer", "J");
  s_game_file_constructor = env->GetMethodID(game_file_class, "<init>", "(J)V");
  env->DeleteLocalRef(game_file_class);

  const jclass game_file_cache_class =
      env->FindClass("org/dolphinemu/dolphinemu/model/GameFileCache");
  s_game_file_cache_class = reinterpret_cast<jclass>(env->NewGlobalRef(game_file_cache_class));
  s_game_file_cache_pointer = env->GetFieldID(game_file_cache_class, "mPointer", "J");
  env->DeleteLocalRef(game_file_cache_class);

  const jclass analytics_class = env->FindClass("org/dolphinemu/dolphinemu/utils/Analytics");
  s_analytics_class = reinterpret_cast<jclass>(env->NewGlobalRef(analytics_class));
  s_send_analytics_report =
      env->GetStaticMethodID(s_analytics_class, "sendReport", "(Ljava/lang/String;[B)V");
  s_get_analytics_value = env->GetStaticMethodID(s_analytics_class, "getValue",
                                                 "(Ljava/lang/String;)Ljava/lang/String;");
  env->DeleteLocalRef(analytics_class);

  const jclass ini_file_class = env->FindClass("org/dolphinemu/dolphinemu/utils/IniFile");
  s_ini_file_class = reinterpret_cast<jclass>(env->NewGlobalRef(ini_file_class));
  s_ini_file_pointer = env->GetFieldID(ini_file_class, "mPointer", "J");
  env->DeleteLocalRef(ini_file_class);

  const jclass ini_file_section_class =
      env->FindClass("org/dolphinemu/dolphinemu/utils/IniFile$Section");
  s_ini_file_section_class = reinterpret_cast<jclass>(env->NewGlobalRef(ini_file_section_class));
  s_ini_file_section_pointer = env->GetFieldID(ini_file_section_class, "mPointer", "J");
  s_ini_file_section_constructor = env->GetMethodID(
      ini_file_section_class, "<init>", "(Lorg/dolphinemu/dolphinemu/utils/IniFile;J)V");
  env->DeleteLocalRef(ini_file_section_class);

  const jclass map_class = env->FindClass("java/util/LinkedHashMap");
  s_linked_hash_map_class = reinterpret_cast<jclass>(env->NewGlobalRef(map_class));
  s_linked_hash_map_init = env->GetMethodID(s_linked_hash_map_class, "<init>", "(I)V");
  s_linked_hash_map_put = env->GetMethodID(
      s_linked_hash_map_class, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
  env->DeleteLocalRef(map_class);

  const jclass compress_cb_class =
      env->FindClass("org/dolphinemu/dolphinemu/utils/CompressCallback");
  s_compress_cb_class = reinterpret_cast<jclass>(env->NewGlobalRef(compress_cb_class));
  s_compress_cb_run = env->GetMethodID(s_compress_cb_class, "run", "(Ljava/lang/String;F)Z");
  env->DeleteLocalRef(compress_cb_class);

  const jclass content_handler_class =
      env->FindClass("org/dolphinemu/dolphinemu/utils/ContentHandler");
  s_content_handler_class = reinterpret_cast<jclass>(env->NewGlobalRef(content_handler_class));
  s_content_handler_open_fd = env->GetStaticMethodID(s_content_handler_class, "openFd",
                                                     "(Ljava/lang/String;Ljava/lang/String;)I");
  s_content_handler_delete =
      env->GetStaticMethodID(s_content_handler_class, "delete", "(Ljava/lang/String;)Z");
  s_content_handler_get_size_and_is_directory = env->GetStaticMethodID(
      s_content_handler_class, "getSizeAndIsDirectory", "(Ljava/lang/String;)J");
  s_content_handler_get_display_name = env->GetStaticMethodID(
      s_content_handler_class, "getDisplayName", "(Ljava/lang/String;)Ljava/lang/String;");
  s_content_handler_get_child_names = env->GetStaticMethodID(
      s_content_handler_class, "getChildNames", "(Ljava/lang/String;Z)[Ljava/lang/String;");
  s_content_handler_do_file_search =
      env->GetStaticMethodID(s_content_handler_class, "doFileSearch",
                             "(Ljava/lang/String;[Ljava/lang/String;Z)[Ljava/lang/String;");
  env->DeleteLocalRef(content_handler_class);

  const jclass network_helper_class =
      env->FindClass("org/dolphinemu/dolphinemu/utils/NetworkHelper");
  s_network_helper_class = reinterpret_cast<jclass>(env->NewGlobalRef(network_helper_class));
  s_network_helper_get_network_ip_address =
      env->GetStaticMethodID(s_network_helper_class, "GetNetworkIpAddress", "()I");
  s_network_helper_get_network_prefix_length =
      env->GetStaticMethodID(s_network_helper_class, "GetNetworkPrefixLength", "()I");
  s_network_helper_get_network_gateway =
      env->GetStaticMethodID(s_network_helper_class, "GetNetworkGateway", "()I");
  env->DeleteLocalRef(network_helper_class);

  const jclass boolean_supplier_class =
      env->FindClass("org/dolphinemu/dolphinemu/utils/BooleanSupplier");
  s_boolean_supplier_class = reinterpret_cast<jclass>(env->NewGlobalRef(boolean_supplier_class));
  s_boolean_supplier_get = env->GetMethodID(s_boolean_supplier_class, "get", "()Z");
  env->DeleteLocalRef(boolean_supplier_class);

  const jclass ar_cheat_class =
      env->FindClass("org/dolphinemu/dolphinemu/features/cheats/model/ARCheat");
  s_ar_cheat_class = reinterpret_cast<jclass>(env->NewGlobalRef(ar_cheat_class));
  s_ar_cheat_pointer = env->GetFieldID(ar_cheat_class, "mPointer", "J");
  s_ar_cheat_constructor = env->GetMethodID(ar_cheat_class, "<init>", "(J)V");
  env->DeleteLocalRef(ar_cheat_class);

  const jclass gecko_cheat_class =
      env->FindClass("org/dolphinemu/dolphinemu/features/cheats/model/GeckoCheat");
  s_gecko_cheat_class = reinterpret_cast<jclass>(env->NewGlobalRef(gecko_cheat_class));
  s_gecko_cheat_pointer = env->GetFieldID(gecko_cheat_class, "mPointer", "J");
  s_gecko_cheat_constructor = env->GetMethodID(gecko_cheat_class, "<init>", "(J)V");
  env->DeleteLocalRef(gecko_cheat_class);

  const jclass patch_cheat_class =
      env->FindClass("org/dolphinemu/dolphinemu/features/cheats/model/PatchCheat");
  s_patch_cheat_class = reinterpret_cast<jclass>(env->NewGlobalRef(patch_cheat_class));
  s_patch_cheat_pointer = env->GetFieldID(patch_cheat_class, "mPointer", "J");
  s_patch_cheat_constructor = env->GetMethodID(patch_cheat_class, "<init>", "(J)V");
  env->DeleteLocalRef(patch_cheat_class);

  const jclass graphics_mod_group_class =
      env->FindClass("org/dolphinemu/dolphinemu/features/cheats/model/GraphicsModGroup");
  s_graphics_mod_group_class =
      reinterpret_cast<jclass>(env->NewGlobalRef(graphics_mod_group_class));
  s_graphics_mod_group_pointer = env->GetFieldID(graphics_mod_group_class, "mPointer", "J");
  s_graphics_mod_group_constructor = env->GetMethodID(graphics_mod_group_class, "<init>", "(J)V");
  env->DeleteLocalRef(graphics_mod_group_class);

  const jclass graphics_mod_class =
      env->FindClass("org/dolphinemu/dolphinemu/features/cheats/model/GraphicsMod");
  s_graphics_mod_class = reinterpret_cast<jclass>(env->NewGlobalRef(graphics_mod_class));
  s_graphics_mod_pointer = env->GetFieldID(graphics_mod_class, "mPointer", "J");
  s_graphics_mod_constructor =
      env->GetMethodID(graphics_mod_class, "<init>",
                       "(JLorg/dolphinemu/dolphinemu/features/cheats/model/GraphicsModGroup;)V");
  env->DeleteLocalRef(graphics_mod_class);

  const jclass riivolution_patches_class =
      env->FindClass("org/dolphinemu/dolphinemu/features/riivolution/model/RiivolutionPatches");
  s_riivolution_patches_class =
      reinterpret_cast<jclass>(env->NewGlobalRef(riivolution_patches_class));
  s_riivolution_patches_pointer = env->GetFieldID(riivolution_patches_class, "mPointer", "J");
  env->DeleteLocalRef(riivolution_patches_class);

  const jclass wii_update_cb_class =
      env->FindClass("org/dolphinemu/dolphinemu/utils/WiiUpdateCallback");
  s_wii_update_cb_class = reinterpret_cast<jclass>(env->NewGlobalRef(wii_update_cb_class));
  s_wii_update_cb_run = env->GetMethodID(s_wii_update_cb_class, "run", "(IIJ)Z");
  env->DeleteLocalRef(wii_update_cb_class);

  return JNI_VERSION;
}

JNIEXPORT void JNI_OnUnload(JavaVM* vm, void* reserved)
{
  JNIEnv* env;
  if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION) != JNI_OK)
    return;

  env->DeleteGlobalRef(s_native_library_class);
  env->DeleteGlobalRef(s_game_file_class);
  env->DeleteGlobalRef(s_game_file_cache_class);
  env->DeleteGlobalRef(s_analytics_class);
  env->DeleteGlobalRef(s_linked_hash_map_class);
  env->DeleteGlobalRef(s_ini_file_class);
  env->DeleteGlobalRef(s_ini_file_section_class);
  env->DeleteGlobalRef(s_compress_cb_class);
  env->DeleteGlobalRef(s_content_handler_class);
  env->DeleteGlobalRef(s_network_helper_class);
  env->DeleteGlobalRef(s_boolean_supplier_class);
  env->DeleteGlobalRef(s_ar_cheat_class);
  env->DeleteGlobalRef(s_gecko_cheat_class);
  env->DeleteGlobalRef(s_patch_cheat_class);
  env->DeleteGlobalRef(s_graphics_mod_group_class);
  env->DeleteGlobalRef(s_graphics_mod_class);
  env->DeleteGlobalRef(s_riivolution_patches_class);
  env->DeleteGlobalRef(s_wii_update_cb_class);
}
}
