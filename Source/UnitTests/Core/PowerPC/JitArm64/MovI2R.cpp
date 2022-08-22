// Copyright 2021 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <cstddef>
#include <type_traits>

#include "Common/Arm64Emitter.h"
#include "Common/Assert.h"
#include "Common/BitUtils.h"
#include "Common/Random.h"

#include <gtest/gtest.h>

namespace
{
using namespace Arm64Gen;

class TestMovI2R : public ARM64CodeBlock
{
public:
  TestMovI2R() { AllocCodeSpace(4096); }

  void Check32(u32 value)
  {
    ResetCodePtr();

    const u8* fn = GetCodePtr();
    {
      const Common::ScopedJITPageWriteAndNoExecute enable_jit_page_writes;
      MOVI2R(ARM64Reg::W0, value);
      RET();
    }

    FlushIcacheSection(const_cast<u8*>(fn), const_cast<u8*>(GetCodePtr()));

    const u64 result = Common::BitCast<u64 (*)()>(fn)();
    EXPECT_EQ(value, result);
  }

  void Check64(u64 value)
  {
    ResetCodePtr();

    const u8* fn = GetCodePtr();
    {
      const Common::ScopedJITPageWriteAndNoExecute enable_jit_page_writes;
      MOVI2R(ARM64Reg::X0, value);
      RET();
    }

    FlushIcacheSection(const_cast<u8*>(fn), const_cast<u8*>(GetCodePtr()));

    const u64 result = Common::BitCast<u64 (*)()>(fn)();
    EXPECT_EQ(value, result);
  }
};

}  // namespace

TEST(JitArm64, MovI2R_32BitValues)
{
  Common::Random::PRNG rng{0};
  TestMovI2R test;
  for (u64 i = 0; i < 0x100000; i++)
  {
    const u32 value = rng.GenerateValue<u32>();
    test.Check32(value);
    test.Check64(value);
  }
}

TEST(JitArm64, MovI2R_Rand)
{
  Common::Random::PRNG rng{0};
  TestMovI2R test;
  for (u64 i = 0; i < 0x100000; i++)
  {
    test.Check64(rng.GenerateValue<u64>());
  }
}

// Construct and test every 64-bit logical immediate
TEST(JitArm64, MovI2R_LogImm)
{
  TestMovI2R test;

  for (unsigned size = 2; size <= 64; size *= 2)
  {
    for (unsigned length = 1; length < size; ++length)
    {
      u64 imm = ~u64{0} >> (64 - length);
      for (unsigned e = size; e < 64; e *= 2)
      {
        imm |= imm << e;
      }
      for (unsigned rotation = 0; rotation < size; ++rotation)
      {
        test.Check64(imm);
        EXPECT_EQ(static_cast<bool>(LogicalImm(imm, 64)), true);

        if (size < 64)
        {
          test.Check32(imm);
          EXPECT_EQ(static_cast<bool>(LogicalImm(static_cast<u32>(imm), 32)), true);
        }

        imm = (imm >> 63) | (imm << 1);
      }
    }
  }
}

TEST(JitArm64, MovI2R_ADP)
{
  TestMovI2R test;
  const u64 base = Common::BitCast<u64>(test.GetCodePtr());

  // Test offsets around 0
  for (s64 i = -0x20000; i < 0x20000; i++)
  {
    const u64 offset = static_cast<u64>(i);
    test.Check64(base + offset);
  }

  // Test offsets around the maximum
  for (const s64 i : {-0x200000ll, 0x200000ll})
  {
    for (s64 j = -4; j < 4; j++)
    {
      const u64 offset = static_cast<u64>(i + j);
      test.Check64(base + offset);
    }
  }
}

TEST(JitArm64, MovI2R_ADRP)
{
  TestMovI2R test;
  const u64 base = Common::BitCast<u64>(test.GetCodePtr()) & ~0xFFF;

  // Test offsets around 0
  for (s64 i = -0x20000; i < 0x20000; i++)
  {
    const u64 offset = static_cast<u64>(i) << 12;
    test.Check64(base + offset);
  }

  // Test offsets around the maximum
  for (const s64 i : {-0x100000000ll, -0x80000000ll, 0x80000000ll, 0x100000000ll})
  {
    for (s64 j = -4; j < 4; j++)
    {
      const u64 offset = static_cast<u64>(i + (j << 12));
      test.Check64(base + offset);
    }
  }
}
