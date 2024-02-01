## @file
# QemuQ35Pkg DSC file used to build host-based unit tests.
#
# Copyright (c) 2019 - 2020, Intel Corporation. All rights reserved.<BR>
# Copyright (C) Microsoft Corporation.
# SPDX-License-Identifier: BSD-2-Clause-Patent
#
##

[Defines]
  PLATFORM_NAME                 = QemuQ35PkgHostTest
  PLATFORM_GUID                 = 1DAA6C82-EEA8-47D6-9062-14B06E6A1BD3
  PLATFORM_VERSION              = 0.1
  DSC_SPECIFICATION             = 0x00010005
  OUTPUT_DIRECTORY              = Build/QemuQ35Pkg/HostTest
  SUPPORTED_ARCHITECTURES       = IA32|X64
  BUILD_TARGETS                 = NOOPT
  SKUID_IDENTIFIER              = DEFAULT
  DEFINE  MFCI_POLICY_EKU_TEST  = "1.3.6.1.4.1.311.45.255.255"

!include UnitTestFrameworkPkg/UnitTestFrameworkPkgHost.dsc.inc
!include MdePkg/MdeLibs.dsc.inc

[PcdsFixedAtBuild]
  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x3f
  gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x80080246
  gEfiMdePkgTokenSpaceGuid.PcdFixedDebugPrintErrorLevel|0x80080246

  # the unit test uses the test certificate that will also be used for testing end-to-end scenarios
  !include MfciPkg/Private/Certs/CA-test.dsc.inc
  gMfciPkgTokenSpaceGuid.PcdMfciPkcs7RequiredLeafEKU  |$(MFCI_POLICY_EKU_TEST)   # use the test version
  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x3f
  gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x80080246
  gEfiMdePkgTokenSpaceGuid.PcdFixedDebugPrintErrorLevel|0x80080246

[LibraryClasses]
  MfciPolicyParsingLib|MfciPkg/Private/Library/MfciPolicyParsingLibNull/MfciPolicyParsingLibNull.inf
  MfciDeviceIdSupportLib|MfciPkg/Library/MfciDeviceIdSupportLibNull/MfciDeviceIdSupportLibNull.inf
  VariablePolicyHelperLib|MdeModulePkg/Library/VariablePolicyHelperLib/VariablePolicyHelperLib.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  UefiRuntimeServicesTableLib|MfciPkg/UnitTests/Library/MockUefiRuntimeServicesTableLib/MockUefiRuntimeServicesTableLib.inf
  DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  ReportStatusCodeLib|MdeModulePkg/Library/DxeReportStatusCodeLib/DxeReportStatusCodeLib.inf
  OemHookStatusCodeLib|MdeModulePkg/Library/OemHookStatusCodeLibNull/OemHookStatusCodeLibNull.inf
  HobLib|MdePkg/Library/DxeHobLib/DxeHobLib.inf
  SerialPortLib|MdePkg/Library/BaseSerialPortLibNull/BaseSerialPortLibNull.inf
  IoLib|MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf
  SafeIntLib|MdePkg/Library/BaseSafeIntLib/BaseSafeIntLib.inf
  FltUsedLib|MdePkg/Library/FltUsedLib/FltUsedLib.inf
  MuTelemetryHelperLib|MsWheaPkg/Library/MuTelemetryHelperLib/MuTelemetryHelperLib.inf
  XmlTreeLib|XmlSupportPkg/Library/XmlTreeLib/XmlTreeLib.inf
  XmlTreeQueryLib|XmlSupportPkg/Library/XmlTreeQueryLib/XmlTreeQueryLib.inf
  FmpDependencyLib|FmpDevicePkg/Library/FmpDependencyLib/FmpDependencyLib.inf
  PeCoffExtraActionLib|MdePkg/Library/BasePeCoffExtraActionLibNull/BasePeCoffExtraActionLibNull.inf
  PeCoffLib|MdePkg/Library/BasePeCoffLib/BasePeCoffLib.inf
  PrmContextBufferLib|PrmPkg/Library/DxePrmContextBufferLib/DxePrmContextBufferLib.inf
  PrmModuleDiscoveryLib|PrmPkg/Library/DxePrmModuleDiscoveryLib/DxePrmModuleDiscoveryLib.inf
  PrmPeCoffLib|PrmPkg/Library/DxePrmPeCoffLib/DxePrmPeCoffLib.inf
  FmpDependencyLib|FmpDevicePkg/Library/FmpDependencyLib/FmpDependencyLib.inf
  SecurityLockAuditLib|MdeModulePkg/Library/SecurityLockAuditLibNull/SecurityLockAuditLibNull.inf
  PerformanceLib|MdePkg/Library/BasePerformanceLibNull/BasePerformanceLibNull.inf
  SvdXmlSettingSchemaSupportLib|SetupDataPkg/Library/SvdXmlSettingSchemaSupportLib/SvdXmlSettingSchemaSupportLib.inf
  SecureBootKeyStoreLib|MsCorePkg/Library/SecureBootKeyStoreLibNull/SecureBootKeyStoreLibNull.inf
  ConfigVariableListLib|SetupDataPkg/Library/ConfigVariableListLib/ConfigVariableListLib.inf
  ConfigSystemModeLib|SetupDataPkg/Test/MockLibrary/MockConfigSystemModeLib/MockConfigSystemModeLib.inf
  ConfigKnobShimLib|SetupDataPkg/Library/ConfigKnobShimLib/ConfigKnobShimDxeLib/ConfigKnobShimDxeLib.inf
  RngLib|MdePkg/Library/BaseRngLib/BaseRngLib.inf
  MmServicesTableLib|MdePkg/Library/MmServicesTableLib/MmServicesTableLib.inf
  SynchronizationLib|MdePkg/Library/BaseSynchronizationLib/BaseSynchronizationLib.inf
  TimerLib|MdePkg/Library/BaseTimerLibNullTemplate/BaseTimerLibNullTemplate.inf
  HobLib|MdeModulePkg/Library/BaseHobLibNull/BaseHobLibNull.inf
  MtrrLib|UefiCpuPkg/Library/MtrrLib/MtrrLib.inf
  CpuPageTableLib|UefiCpuPkg/Library/CpuPageTableLib/CpuPageTableLib.inf

[LibraryClasses.X64]
  TimerLib|MdePkg/Library/BaseTimerLibNullTemplate/BaseTimerLibNullTemplate.inf
  Tpm2CommandLib|SecurityPkg/Library/Tpm2CommandLib/Tpm2CommandLib.inf
  Tpm2DeviceLib|SecurityPkg/Library/Tpm2DeviceLibDTpm/Tpm2DeviceLibDTpm.inf
  Tpm2DebugLib|SecurityPkg/Library/Tpm2DebugLib/Tpm2DebugLibNull.inf
  MfciRetrievePolicyLib|MfciPkg/Library/MfciRetrievePolicyLibNull/MfciRetrievePolicyLibNull.inf

[Components]
MfciPkg/MfciDxe/Test/MfciTargetingHostTest.inf
MfciPkg/MfciDxe/Test/MfciVerifyPolicyAndChangeHostTest.inf {
  <LibraryClasses>
    ResetUtilityLib|MfciPkg/UnitTests/Library/MockResetUtilityLib/MockResetUtilityLib.inf
    BaseCryptLib|MfciPkg/UnitTests/Library/MockBaseCryptLib/MockBaseCryptLib.inf
}
MfciPkg/MfciDxe/Test/MfciPublicInterfaceHostTest.inf
MfciPkg/MfciDxe/Test/MfciMultipleCertsHostTest.inf
MsWheaPkg/MsWheaReport/Test/MsWheaReportCommonHostTest.inf {
  <PcdsFixedAtBuild>
    gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x07
}
MsWheaPkg/MsWheaReport/Test/MsWheaReportHERHostTest.inf {
  <PcdsFixedAtBuild>
    gMsWheaPkgTokenSpaceGuid.PcdDeviceIdentifierGuid|{0x16, 0x33, 0x43, 0x92, 0xA2, 0x00, 0x43, 0xEE, 0xBF, 0x63, 0x7F, 0x41, 0xEA, 0x3C, 0xEA, 0xAB}
}
SetupDataPkg/Library/ConfigVariableListLib/UnitTest/ConfigVariableListLibUnitTest.inf
SetupDataPkg/Library/ConfigKnobShimLib/ConfigKnobShimDxeLib/UnitTest/ConfigKnobShimDxeLibUnitTest.inf {
  <LibraryClasses>
    UefiRuntimeServicesTableLib|SetupDataPkg/Test/MockLibrary/MockUefiRuntimeServicesTableLib/MockUefiRuntimeServicesTableLib.inf
}
SetupDataPkg/Library/ConfigKnobShimLib/ConfigKnobShimPeiLib/UnitTest/ConfigKnobShimPeiLibUnitTest.inf {
  <LibraryClasses>
    ConfigKnobShimLib|SetupDataPkg/Library/ConfigKnobShimLib/ConfigKnobShimPeiLib/ConfigKnobShimPeiLib.inf
    PeiServicesLib|SetupDataPkg/Test/MockLibrary/MockPeiServicesLib/MockPeiServicesLib.inf
}
SetupDataPkg/Library/ConfigKnobShimLib/ConfigKnobShimMmLib/UnitTest/ConfigKnobShimMmLibUnitTest.inf {
  <LibraryClasses>
    ConfigKnobShimLib|SetupDataPkg/Library/ConfigKnobShimLib/ConfigKnobShimMmLib/ConfigKnobShimMmLib.inf
    MmServicesTableLib|SetupDataPkg/Test/MockLibrary/MockMmServicesTableLib/MockMmServicesTableLib.inf
}
SetupDataPkg/ConfApp/UnitTest/ConfAppUnitTest.inf {
  <LibraryClasses>
    UefiBootServicesTableLib|SetupDataPkg/Test/MockLibrary/MockUefiBootServicesTableLib/MockUefiBootServicesTableLib.inf
    UefiRuntimeServicesTableLib|SetupDataPkg/Test/MockLibrary/MockUefiRuntimeServicesTableLib/MockUefiRuntimeServicesTableLib.inf
    ResetSystemLib|SetupDataPkg/Test/MockLibrary/MockResetSystemLib/MockResetSystemLib.inf
}
SetupDataPkg/ConfApp/UnitTest/ConfAppSysInfoUnitTest.inf {
  <LibraryClasses>
    UefiBootServicesTableLib|SetupDataPkg/Test/MockLibrary/MockUefiBootServicesTableLib/MockUefiBootServicesTableLib.inf
    UefiRuntimeServicesTableLib|SetupDataPkg/Test/MockLibrary/MockUefiRuntimeServicesTableLib/MockUefiRuntimeServicesTableLib.inf
    ResetSystemLib|SetupDataPkg/Test/MockLibrary/MockResetSystemLib/MockResetSystemLib.inf
}
SetupDataPkg/ConfApp/UnitTest/ConfAppBootOptionUnitTest.inf {
  <LibraryClasses>
    UefiBootServicesTableLib|SetupDataPkg/Test/MockLibrary/MockUefiBootServicesTableLib/MockUefiBootServicesTableLib.inf
    UefiRuntimeServicesTableLib|SetupDataPkg/Test/MockLibrary/MockUefiRuntimeServicesTableLib/MockUefiRuntimeServicesTableLib.inf
    ResetSystemLib|SetupDataPkg/Test/MockLibrary/MockResetSystemLib/MockResetSystemLib.inf
}
SetupDataPkg/ConfApp/UnitTest/ConfAppSetupConfUnitTest.inf {
  <LibraryClasses>
    UefiBootServicesTableLib|SetupDataPkg/Test/MockLibrary/MockUefiBootServicesTableLib/MockUefiBootServicesTableLib.inf
    UefiRuntimeServicesTableLib|SetupDataPkg/Test/MockLibrary/MockUefiRuntimeServicesTableLib/MockUefiRuntimeServicesTableLib.inf
    ResetSystemLib|SetupDataPkg/Test/MockLibrary/MockResetSystemLib/MockResetSystemLib.inf
  <PcdsFixedAtBuild>
    gSetupDataPkgTokenSpaceGuid.PcdConfigurationPolicyGuid|{GUID("00000000-0000-0000-0000-000000000000")}
}
SetupDataPkg/ConfApp/UnitTest/ConfAppSecureBootUnitTest.inf {
  <LibraryClasses>
    UefiBootServicesTableLib|SetupDataPkg/Test/MockLibrary/MockUefiBootServicesTableLib/MockUefiBootServicesTableLib.inf
    UefiRuntimeServicesTableLib|SetupDataPkg/Test/MockLibrary/MockUefiRuntimeServicesTableLib/MockUefiRuntimeServicesTableLib.inf
    ResetSystemLib|SetupDataPkg/Test/MockLibrary/MockResetSystemLib/MockResetSystemLib.inf
}
MdeModulePkg/Universal/Variable/RuntimeDxe/RuntimeDxeUnitTest/VariableLockRequestToLockUnitTest.inf {
  <LibraryClasses>
    VariablePolicyLib|MdeModulePkg/Library/VariablePolicyLib/VariablePolicyLib.inf
    VariablePolicyHelperLib|MdeModulePkg/Library/VariablePolicyHelperLib/VariablePolicyHelperLib.inf
  <PcdsFixedAtBuild>
    gEfiMdeModulePkgTokenSpaceGuid.PcdAllowVariablePolicyEnforcementDisable|TRUE
}
MdeModulePkg/Universal/Variable/RuntimeDxe/RuntimeDxeUnitTest/VariableRuntimeDxeUnitTest.inf {
  <LibraryClasses>
    UefiLib|MdePkg/Test/Library/StubUefiLib/StubUefiLib.inf
    VariablePolicyLib|MdeModulePkg/Library/VariablePolicyLib/VariablePolicyLib.inf
    VariablePolicyHelperLib|MdeModulePkg/Library/VariablePolicyHelperLib/VariablePolicyHelperLib.inf
    TpmMeasurementLib|MdeModulePkg/Library/TpmMeasurementLibNull/TpmMeasurementLibNull.inf
    AuthVariableLib|MdeModulePkg/Library/AuthVariableLibNull/AuthVariableLibNull.inf
    DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
    UefiRuntimeLib|MdePkg/Library/UefiRuntimeLib/UefiRuntimeLib.inf
    DxeServicesTableLib|MdePkg/Library/DxeServicesTableLib/DxeServicesTableLib.inf
    UefiRuntimeServicesTableLib|MdePkg/Test/Library/MockUefiRuntimeServicesTableLib/MockUefiRuntimeServicesTableLib.inf
    UefiBootServicesTableLib|MdePkg/Test/Library/MockUefiBootServicesTableLib/MockUefiBootServicesTableLib.inf
    SynchronizationLib|MdePkg/Test/Library/SynchronizationLibHostUnitTest/SynchronizationLibHostUnitTest.inf
    VariableFlashInfoLib|MdeModulePkg/Library/BaseVariableFlashInfoLib/BaseVariableFlashInfoLib.inf
    HobLib|MdePkg/Test/Library/StubHobLib/StubHobLib.inf

    VarCheckLib|MdeModulePkg/Library/VarCheckLib/VarCheckLib.inf
    NULL|MdeModulePkg/Library/VarCheckUefiLib/VarCheckUefiLib.inf
    NULL|MdeModulePkg/Library/VarCheckPolicyLib/VarCheckPolicyLibVariableDxe.inf

  <PcdsFixedAtBuild>
    gEfiMdeModulePkgTokenSpaceGuid.PcdAllowVariablePolicyEnforcementDisable|TRUE
    gEfiMdeModulePkgTokenSpaceGuid.PcdEmuVariableNvModeEnable|TRUE
    # SCT tests are noisy, so disable VERBOSE.
    gUnitTestFrameworkPkgTokenSpaceGuid.PcdUnitTestLogLevel|0x00000007
}
MdeModulePkg/Bus/Pci/NvmExpressDxe/UnitTest/MediaSanitizeUnitTestHost.inf
MdeModulePkg/Core/Dxe/UnitTest/MemoryProtectionUnitTestHost.inf {
  <LibraryClasses>
    HobLib|MdeModulePkg/Library/BaseHobLibNull/BaseHobLibNull.inf
    PeCoffGetEntryPointLib|MdePkg/Library/BasePeCoffGetEntryPointLib/BasePeCoffGetEntryPointLib.inf
    DxeMemoryProtectionHobLib|MdeModulePkg/Library/MemoryProtectionHobLibNull/DxeMemoryProtectionHobLibNull.inf
    PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
    UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
    UefiDecompressLib|MdePkg/Library/BaseUefiDecompressLib/BaseUefiDecompressLib.inf
    PerformanceLib|MdePkg/Library/BasePerformanceLibNull/BasePerformanceLibNull.inf
    UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
    PeCoffLib|MdePkg/Library/BasePeCoffLib/BasePeCoffLib.inf
    PeCoffExtraActionLib|MdePkg/Library/BasePeCoffExtraActionLibNull/BasePeCoffExtraActionLibNull.inf
    ExtractGuidedSectionLib|MdePkg/Library/PeiExtractGuidedSectionLib/PeiExtractGuidedSectionLib.inf
    DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
    ReportStatusCodeLib|MdePkg/Library/BaseReportStatusCodeLibNull/BaseReportStatusCodeLibNull.inf
    DxeServicesLib|MdePkg/Library/DxeServicesLib/DxeServicesLib.inf
    DebugAgentLib|MdeModulePkg/Library/DebugAgentLibNull/DebugAgentLibNull.inf
    CpuExceptionHandlerLib|MdeModulePkg/Library/CpuExceptionHandlerLibNull/CpuExceptionHandlerLibNull.inf
    UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
    MemoryBinOverrideLib|MdeModulePkg/Library/MemoryBinOverrideLibNull/MemoryBinOverrideLibNull.inf
  <PcdsFixedAtBuild>
    gEfiMdeModulePkgTokenSpaceGuid.PcdLoadModuleAtFixAddressEnable|0
    gEfiMdeModulePkgTokenSpaceGuid.PcdLoadFixAddressRuntimeCodePageNumber|0
    gEfiMdeModulePkgTokenSpaceGuid.PcdLoadFixAddressBootTimeCodePageNumber|0
}
MdePkg/Test/UnitTest/Library/BaseSafeIntLib/TestBaseSafeIntLibHost.inf
PolicyServicePkg/PolicyService/DxeMm/UnitTest/DxeMmPolicyUnitTest.inf
PolicyServicePkg/PolicyService/Pei/UnitTest/PeiPolicyUnitTest.inf
SetupDataPkg/Library/ConfigKnobShimLib/ConfigKnobShimDxeLib/GoogleTest/ConfigKnobShimDxeLibGoogleTest.inf {
  <LibraryClasses>
    UefiRuntimeServicesTableLib|MdePkg/Test/Mock/Library/GoogleTest/MockUefiRuntimeServicesTableLib/MockUefiRuntimeServicesTableLib.inf
}
SetupDataPkg/Library/ConfigKnobShimLib/ConfigKnobShimPeiLib/GoogleTest/ConfigKnobShimPeiLibGoogleTest.inf {
  <LibraryClasses>
    ConfigKnobShimLib|SetupDataPkg/Library/ConfigKnobShimLib/ConfigKnobShimPeiLib/ConfigKnobShimPeiLib.inf
    PeiServicesLib|MdePkg/Test/Mock/Library/GoogleTest/MockPeiServicesLib/MockPeiServicesLib.inf
}

[BuildOptions]
  *_*_*_CC_FLAGS            = -D DISABLE_NEW_DEPRECATED_INTERFACES
