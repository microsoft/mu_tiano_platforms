## @file
#  Open Virtual Machine Firmware for the Q35 platform using Project Mu
#
#  Copyright (c) Microsoft Corporation
#  Copyright (c) 2006 - 2019, Intel Corporation. All rights reserved.<BR>
#  (C) Copyright 2016 Hewlett Packard Enterprise Development LP<BR>
#
#  SPDX-License-Identifier: BSD-2-Clause-Patent
#
##

################################################################################
#
# Defines Section - statements that will be processed to create a Makefile.
#
################################################################################
[Defines]
  PLATFORM_NAME                  = QemuQ35
  PLATFORM_GUID                  = 163b507c-8702-496a-99d7-566c36e14728
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/QemuQ35X64Pkg
  SUPPORTED_ARCHITECTURES        = X64
  PEI_CRYPTO_ARCH                = X64

!include QemuQ35Pkg/QemuQ35PkgCommon.dsc.inc

[Components.X64]
!include QemuQ35Pkg/QemuQ35PkgCommonPei.dsc.inc

################################################################################
#
# Build Options
#
################################################################################
[BuildOptions]
  GCC:RELEASE_*_*_CC_FLAGS             = -DMDEPKG_NDEBUG
  MSFT:RELEASE_*_*_CC_FLAGS            = /D MDEPKG_NDEBUG

  # Exception tables are required for stack walks in the debugger.
  MSFT:*_*_X64_GENFW_FLAGS  = --keepexceptiontable
  GCC:*_*_X64_GENFW_FLAGS   = --keepexceptiontable

  #
  # Disable deprecated APIs.
  #
  MSFT:*_*_*_CC_FLAGS = /D DISABLE_NEW_DEPRECATED_INTERFACES
  GCC:*_*_*_CC_FLAGS = -D DISABLE_NEW_DEPRECATED_INTERFACES

[BuildOptions.common.PEIM, BuildOptions.common.PEI_CORE, BuildOptions.common.SEC]
  MSFT:*_*_*_DLINK_FLAGS = /ALIGN:64
  GCC:*_GCC5_*_DLINK_FLAGS = -z common-page-size=64
  GCC:*_CLANGPDB_*_DLINK_FLAGS = /ALIGN:64

# Force PE/COFF sections to be aligned at 4KB boundaries to support page level
# protection of DXE_SMM_DRIVER/SMM_CORE modules
[BuildOptions.common.EDKII.DXE_SMM_DRIVER, BuildOptions.common.EDKII.DXE_RUNTIME_DRIVER, BuildOptions.common.EDKII.SMM_CORE, BuildOptions.common.EDKII.DXE_DRIVER, BuildOptions.common.EDKII.DXE_CORE, BuildOptions.common.EDKII.UEFI_DRIVER, BuildOptions.common.EDKII.UEFI_APPLICATION, BuildOptions.common.EDKII.MM_CORE_STANDALONE, BuildOptions.common.EDKII.MM_STANDALONE]
  MSFT:*_*_*_DLINK_FLAGS = /ALIGN:4096
  GCC:*_GCC5_*_DLINK_FLAGS = -z common-page-size=0x1000
  GCC:*_CLANGPDB_*_DLINK_FLAGS = /ALIGN:4096
