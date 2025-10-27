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

QemuQ35Pkg/ResetVector/ResetVector.inf

  #########################################
  # SEC Phase modules
  #########################################
  QemuQ35Pkg/Sec/SecMain.inf {
    <LibraryClasses>
      NULL|MdeModulePkg/Library/LzmaCustomDecompressLib/LzmaCustomDecompressLib.inf
  }

  #########################################
  # PEI Phase modules
  #########################################
  MdeModulePkg/Core/Pei/PeiMain.inf
  MdeModulePkg/Universal/PCD/Pei/Pcd.inf  {
    <LibraryClasses>
      PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  }
  MdeModulePkg/Universal/ReportStatusCodeRouter/Pei/ReportStatusCodeRouterPei.inf {
    <LibraryClasses>
      PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  }
  MdeModulePkg/Universal/StatusCodeHandler/Pei/StatusCodeHandlerPei.inf {
    <LibraryClasses>
      PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  }
  MdeModulePkg/Core/DxeIplPeim/DxeIpl.inf

!if $(TPM_REPLAY_ENABLED) == TRUE
  TpmTestingPkg/TpmReplayPei/Pei/TpmReplayPei.inf {
    <LibraryClasses>
      FvMeasurementExclusionLib|QemuQ35Pkg/Library/PeiFvMeasurementExclusionLib/PeiFvMeasurementExclusionLib.inf
      InputChannelLib|QemuPkg/Library/BaseFwCfgInputChannelLib/BaseFwCfgInputChannelLib.inf
      Tpm2DeviceLib|SecurityPkg/Library/Tpm2DeviceLibDTpm/Tpm2DeviceLibDTpm.inf
    <PcdsPatchableInModule>
      gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x3F
    <PcdsFixedAtBuild>
      gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x80480246
  }
!endif

  SecurityPkg/RandomNumberGenerator/RngPei/RngPei.inf {
    <LibraryClasses>
      RngLib|MdeModulePkg/Library/BaseRngLibTimerLib/BaseRngLibTimerLib.inf
  }

  QemuQ35Pkg/PlatformPei/PlatformPei.inf {
    <LibraryClasses>
      NULL|StandaloneMmPkg/Library/PeiStandaloneMmHobProductionLib/PeiStandaloneMmHobProductionLib.inf
  }
  MdeModulePkg/Universal/FaultTolerantWritePei/FaultTolerantWritePei.inf
  MdeModulePkg/Universal/Variable/Pei/VariablePei.inf
  QemuQ35Pkg/SmmAccess/SmmAccessPei.inf
  MmSupervisorPkg/Drivers/StandaloneMmHob/StandaloneMmHob.inf
  MmSupervisorPkg/Drivers/MmCommunicationBuffer/MmCommunicationBufferPei.inf
!if $(PEI_MM_IPL_ENABLED) == TRUE
  MmSupervisorPkg/Drivers/MmPeiLaunchers/MmIplPei.inf
  QemuQ35Pkg/SmmControl2Dxe/MmControlPei.inf
!endif

  UefiCpuPkg/CpuMpPei/CpuMpPei.inf

!if $(TPM_ENABLE) == TRUE
  QemuPkg/Tcg/Tcg2Config/Tcg2ConfigPei.inf
  SecurityPkg/Tcg/Tcg2Pei/Tcg2Pei.inf {
    <LibraryClasses>
      HashLib|SecurityPkg/Library/HashLibBaseCryptoRouter/HashLibBaseCryptoRouterPei.inf
      NULL|SecurityPkg/Library/HashInstanceLibSha1/HashInstanceLibSha1.inf
      NULL|SecurityPkg/Library/HashInstanceLibSha256/HashInstanceLibSha256.inf
      NULL|SecurityPkg/Library/HashInstanceLibSha384/HashInstanceLibSha384.inf
      NULL|SecurityPkg/Library/HashInstanceLibSha512/HashInstanceLibSha512.inf
      NULL|SecurityPkg/Library/HashInstanceLibSm3/HashInstanceLibSm3.inf
  }
!endif

  #
  # MU Modules
  #
  ## PEI
  MdeModulePkg/Universal/ResetSystemPei/ResetSystemPei.inf
  MsCorePkg/Universal/StatusCodeHandler/Serial/Pei/SerialStatusCodeHandlerPei.inf {
    <LibraryClasses>
      DebugLib|MdePkg/Library/BaseDebugLibSerialPort/BaseDebugLibSerialPort.inf
  }

  MsCorePkg/Core/GuidedSectionExtractPeim/GuidedSectionExtract.inf {
    <LibraryClasses>
    NULL|MdeModulePkg/Library/LzmaCustomDecompressLib/LzmaCustomDecompressLib.inf
  }
  MsWheaPkg/MsWheaReport/Pei/MsWheaReportPei.inf

  MsGraphicsPkg/MsUiTheme/Pei/MsUiThemePpi.inf
  MsGraphicsPkg/MsEarlyGraphics/Pei/MsEarlyGraphics.inf
  MdeModulePkg/Universal/Acpi/FirmwarePerformanceDataTablePei/FirmwarePerformancePei.inf
  OemPkg/DeviceStatePei/DeviceStatePei.inf
  MfciPkg/MfciPei/MfciPei.inf

  PolicyServicePkg/PolicyService/Pei/PolicyPei.inf
  QemuQ35Pkg/ConfigKnobs/ConfigKnobs.inf
  OemPkg/OemConfigPolicyCreatorPei/OemConfigPolicyCreatorPei.inf {
    <LibraryClasses>
      # producer of config data
      NULL|QemuQ35Pkg/Library/Q35ConfigDataLib/Q35ConfigDataLib.inf
  }

  DebuggerFeaturePkg/DebugConfigPei/DebugConfigPei.inf
