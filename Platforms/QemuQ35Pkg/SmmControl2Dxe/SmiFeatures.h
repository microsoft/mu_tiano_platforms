/**@file
  Negotiate SMI features with QEMU, and configure UefiCpuPkg/PiSmmCpuDxeSmm
  accordingly.

  Copyright (C) 2016-2017, Red Hat, Inc.
  Copyright (c) Microsoft Corporation

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef __SMI_FEATURES_H__
#define __SMI_FEATURES_H__

/**
  Negotiate SMI features with QEMU.

  @retval FALSE  It is not an error if SMI feature negotiation is not supported
                 by QEMU. It just means the data cannot be used.

  @retval TRUE   SMI feature negotiation is supported, and it has completed
                 successfully as well (failure to negotiate is a fatal error
                 and the function never returns in that case).
**/
BOOLEAN
NegotiateSmiFeatures (
  VOID
  );

#endif
