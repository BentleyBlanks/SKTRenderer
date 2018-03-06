#pragma once

#include "windows.h"
#include <intrin.h>
#include "RBBasedata.h"
//¶ÔÆë
#define ALIGN(n) __declspec(align(n))

#define DLLEXPORT __declspec(dllexport)
#define DLLIMPORT __declspec(dllimport)


struct PlatformMath
{

#ifdef _WIN32
#pragma intrinsic( _BitScanReverse )
static FORCEINLINE uint32 floor_log2(uint32 Value)
{
  // Use BSR to return the log2 of the integer
  DWORD Log2;
  if (_BitScanReverse(&Log2, Value) != 0)
  {
    return Log2;
  }

  return 0;
}
static FORCEINLINE uint32 count_leading_zeros(uint32 Value)
{
  // Use BSR to return the log2 of the integer
  DWORD Log2;
  if (_BitScanReverse(&Log2, Value) != 0)
  {
    return 31 - Log2;
  }

  return 32;
}
static FORCEINLINE uint32 count_trailing_zeros(uint32 Value)
{
  if (Value == 0)
  {
    return 32;
  }
  uint32 BitIndex;	// 0-based, where the LSB is 0 and MSB is 31
  _BitScanForward((::DWORD *)&BitIndex, Value);	// Scans from LSB to MSB
  return BitIndex;
}
static FORCEINLINE uint32 ceil_log_two(uint32 Arg)
{
  int32 Bitmask = ((int32)(count_leading_zeros(Arg) << 26)) >> 31;
  return (32 - count_leading_zeros(Arg - 1)) & (~Bitmask);
}
static FORCEINLINE uint32 round_up_to_power_of_two(uint32 Arg)
{
  return 1 << ceil_log_two(Arg);
}
#else
static FORCEINLINE uint32 floor_log2(uint32 Value)
{
  return 0;
}
static FORCEINLINE uint32 count_leading_zeros(uint32 Value)
{
  return 32;
}
static FORCEINLINE uint32 count_trailing_zeros(uint32 Value)
{
  return 1;
}
static FORCEINLINE uint32 ceil_log_two(uint32 Arg)
{
  return 1;
}
static FORCEINLINE uint32 round_up_to_power_of_two(uint32 Arg)
{
  return 1 << 1;
}
#endif
};