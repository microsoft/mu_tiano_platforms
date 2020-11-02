/** @file -- RustExtraIntrinsicsLib.c
Extra intrinsics that aren't included in BaseIntrinsics to supplement what would
normally be provided by the compiler_intrinsics crate.
This is to avoid competition between the two.

Copyright (c) Microsoft Corporation.
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>

#include <Library/BaseMemoryLib.h>

void * memmove (void *dest, const void *src, unsigned int count)
{
    // Internal CopyMem should fulfill all the promises of memmove.
    return CopyMem (dest, src, (UINTN)count);
}

void * memcpy (void *dest, const void *src, unsigned int count)
{
    // Internal CopyMem should fulfill all the promises of memmove.
    return CopyMem (dest, src, (UINTN)count);
}

int memcmp (const void *buf1, const void *buf2, size_t count)
{
  return (int)CompareMem(buf1, buf2, count);
}

void * memset (void *dest, int ch, size_t count)
{
  //
  // NOTE: Here we use one base implementation for memset, instead of the direct
  //       optimized SetMem() wrapper. Because the IntrinsicLib has to be built
  //       without whole program optimization option, and there will be some
  //       potential register usage errors when calling other optimized codes.
  //

  //
  // Declare the local variables that actually move the data elements as
  // volatile to prevent the optimizer from replacing this function with
  // the intrinsic memset()
  //
  volatile UINT8  *Pointer;

  Pointer = (UINT8 *)dest;
  while (count-- != 0) {
    *(Pointer++) = (UINT8)ch;
  }

  return dest;
}
