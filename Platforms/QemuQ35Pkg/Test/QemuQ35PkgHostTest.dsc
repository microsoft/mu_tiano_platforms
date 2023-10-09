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

[PcdsPatchableInModule]
  gUefiCpuPkgTokenSpaceGuid.PcdCpuNumberOfReservedVariableMtrrs|0

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
  OpensslLib|CryptoPkg/Library/OpensslLib/OpensslLib.inf
  BaseCryptLib|CryptoPkg/Library/BaseCryptLib/UnitTestHostBaseCryptLib.inf
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
HidPkg/HidMouseAbsolutePointerDxe/UnitTest/HidMouseAbsolutePointerHostTest.inf {
  <LibraryClasses>
    UefiLib|MdePkg/Test/Library/StubUefiLib/StubUefiLib.inf
    UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  <PcdsFixedAtBuild>
    #Turn off Halt on Assert and Print Assert so that libraries can
    #be tested in more of a release mode environment
    gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x0E
}
MfciPkg/UnitTests/Library/MockResetUtilityLib/MockResetUtilityLib.inf
MfciPkg/UnitTests/Library/MockBaseCryptLib/MockBaseCryptLib.inf
MfciPkg/UnitTests/Library/MockUefiRuntimeServicesTableLib/MockUefiRuntimeServicesTableLib.inf
MfciPkg/UnitTests/Library/MockMfciRetrieveTargetPolicyLib/MockMfciRetrieveTargetPolicyLib.inf
MfciPkg/MfciDxe/Test/MfciTargetingHostTest.inf
MfciPkg/MfciDxe/Test/MfciVerifyPolicyAndChangeHostTest.inf {
  <LibraryClasses>
    ResetUtilityLib|MfciPkg/UnitTests/Library/MockResetUtilityLib/MockResetUtilityLib.inf
    BaseCryptLib|MfciPkg/UnitTests/Library/MockBaseCryptLib/MockBaseCryptLib.inf
}
MfciPkg/MfciDxe/Test/MfciVerifyPolicyAndChangeRoTHostTest.inf {
  <LibraryClasses>
    ResetUtilityLib|MfciPkg/UnitTests/Library/MockResetUtilityLib/MockResetUtilityLib.inf
    MfciRetrieveTargetPolicyLib|MfciPkg/UnitTests/Library/MockMfciRetrieveTargetPolicyLib/MockMfciRetrieveTargetPolicyLib.inf
}
MfciPkg/MfciDxe/Test/MfciPublicInterfaceHostTest.inf
MfciPkg/MfciDxe/Test/MfciMultipleCertsHostTest.inf
MsCorePkg/MacAddressEmulationDxe/Test/MacAddressEmulationDxeHostTest.inf
MsWheaPkg/MsWheaReport/Test/MsWheaReportCommonHostTest.inf {
  <PcdsFixedAtBuild>
    gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x07
}
MsWheaPkg/MsWheaReport/Test/MsWheaReportHERHostTest.inf {
  <PcdsFixedAtBuild>
    gMsWheaPkgTokenSpaceGuid.PcdDeviceIdentifierGuid|{0x16, 0x33, 0x43, 0x92, 0xA2, 0x00, 0x43, 0xEE, 0xBF, 0x63, 0x7F, 0x41, 0xEA, 0x3C, 0xEA, 0xAB}
}
MsWheaPkg/Test/UnitTests/Library/MuTelemetryHelperLib/MuTelemetryHelperLibHostTest.inf {
  <LibraryClasses>
    MuTelemetryHelperLib|MsWheaPkg/Library/MuTelemetryHelperLib/MuTelemetryHelperLib.inf
    ReportStatusCodeLib|MsWheaPkg/Library/MuTelemetryHelperLib/MuTelemetryHelperLib.inf
}
XmlSupportPkg/Test/UnitTest/XmlTreeLib/XmlTreeLibUnitTestsHost.inf
XmlSupportPkg/Test/UnitTest/XmlTreeQueryLib/XmlTreeQueryLibUnitTestsHost.inf {
  <PcdsFixedAtBuild>
  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x0E
}
PrmPkg/Library/DxePrmContextBufferLib/UnitTest/DxePrmContextBufferLibUnitTestHost.inf
PrmPkg/Library/DxePrmModuleDiscoveryLib/UnitTest/DxePrmModuleDiscoveryLibUnitTestHost.inf
SecurityPkg/Library/SecureBootVariableLib/UnitTest/MockUefiRuntimeServicesTableLib.inf
SecurityPkg/Library/SecureBootVariableLib/UnitTest/MockPlatformPKProtectionLib.inf
SecurityPkg/Library/SecureBootVariableLib/UnitTest/MockUefiLib.inf
SecurityPkg/Test/Mock/Library/GoogleTest/MockPlatformPKProtectionLib/MockPlatformPKProtectionLib.inf
SecurityPkg/Library/SecureBootVariableLib/UnitTest/SecureBootVariableLibUnitTest.inf {
  <LibraryClasses>
    SecureBootVariableLib|SecurityPkg/Library/SecureBootVariableLib/SecureBootVariableLib.inf
    UefiRuntimeServicesTableLib|SecurityPkg/Library/SecureBootVariableLib/UnitTest/MockUefiRuntimeServicesTableLib.inf
    PlatformPKProtectionLib|SecurityPkg/Library/SecureBootVariableLib/UnitTest/MockPlatformPKProtectionLib.inf
    UefiLib|SecurityPkg/Library/SecureBootVariableLib/UnitTest/MockUefiLib.inf
}
SecurityPkg/Library/SecureBootVariableLib/GoogleTest/SecureBootVariableLibGoogleTest.inf {
  <LibraryClasses>
    SecureBootVariableLib|SecurityPkg/Library/SecureBootVariableLib/SecureBootVariableLib.inf
    UefiRuntimeServicesTableLib|MdePkg/Test/Mock/Library/GoogleTest/MockUefiRuntimeServicesTableLib/MockUefiRuntimeServicesTableLib.inf
    PlatformPKProtectionLib|SecurityPkg/Test/Mock/Library/GoogleTest/MockPlatformPKProtectionLib/MockPlatformPKProtectionLib.inf
    UefiLib|MdePkg/Test/Mock/Library/GoogleTest/MockUefiLib/MockUefiLib.inf
}
SetupDataPkg/Test/MockLibrary/MockUefiRuntimeServicesTableLib/MockUefiRuntimeServicesTableLib.inf
SetupDataPkg/Test/MockLibrary/MockUefiBootServicesTableLib/MockUefiBootServicesTableLib.inf
SetupDataPkg/Test/MockLibrary/MockResetSystemLib/MockResetSystemLib.inf
SetupDataPkg/Test/MockLibrary/MockResetUtilityLib/MockResetUtilityLib.inf
SetupDataPkg/Test/MockLibrary/MockPcdLib/MockPcdLib.inf
SetupDataPkg/Test/MockLibrary/MockConfigSystemModeLib/MockConfigSystemModeLib.inf
SetupDataPkg/Test/MockLibrary/MockPeiServicesLib/MockPeiServicesLib.inf
SetupDataPkg/Test/MockLibrary/MockActiveProfileIndexSelectorLib/MockActiveProfileIndexSelectorLib.inf
SetupDataPkg/Test/MockLibrary/MockMmServicesTableLib/MockMmServicesTableLib.inf
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
MmSupervisorPkg/Library/SmmPolicyGateLib/UnitTest/SmmPolicyGateLibUnitTest.inf {
  <LibraryClasses>
    SmmPolicyGateLib|MmSupervisorPkg/Library/SmmPolicyGateLib/SmmPolicyGateLib.inf
}
CryptoPkg/Test/UnitTest/Library/BaseCryptLib/TestBaseCryptLibHost.inf {
  <LibraryClasses>
    OpensslLib|CryptoPkg/Library/OpensslLib/OpensslLibFull.inf
}
CryptoPkg/Test/UnitTest/Library/BaseCryptLib/TestBaseCryptLibHost.inf {
  <Defines>
    FILE_GUID = 3604CCB8-138C-488F-8045-18704F73E734
  <LibraryClasses>
    OpensslLib|CryptoPkg/Library/OpensslLib/OpensslLibFullAccel.inf
}
MdeModulePkg/Library/DxeResetSystemLib/UnitTest/DxeResetSystemLibUnitTestHost.inf {
  <LibraryClasses>
    ResetSystemLib|MdeModulePkg/Library/DxeResetSystemLib/DxeResetSystemLib.inf
    UefiRuntimeServicesTableLib|MdePkg/Test/Library/MockUefiRuntimeServicesTableLib/MockUefiRuntimeServicesTableLib.inf
}

MdeModulePkg/Universal/Variable/RuntimeDxe/RuntimeDxeUnitTest/VariableLockRequestToLockUnitTest.inf {
  <LibraryClasses>
    VariablePolicyLib|MdeModulePkg/Library/VariablePolicyLib/VariablePolicyLib.inf
    VariablePolicyHelperLib|MdeModulePkg/Library/VariablePolicyHelperLib/VariablePolicyHelperLib.inf
  <PcdsFixedAtBuild>
    gEfiMdeModulePkgTokenSpaceGuid.PcdAllowVariablePolicyEnforcementDisable|TRUE
}

# MU_CHANGE [BEGIN] - Add a host-based unit test for common variable services code.
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
# MU_CHANGE [END] - Add a host-based unit test for common variable services code.

MdeModulePkg/Library/UefiSortLib/UnitTest/UefiSortLibUnitTest.inf {
  <LibraryClasses>
    SortLib|MdeModulePkg/Library/UefiSortLib/UefiSortLib.inf
    DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
    # MU_CHANGE
    # The UefiBootServicesTableLib cannot be used in a generic way, but is
    # safe for this module because it only needs the symbol defined. gBS is
    # never actually used.
    UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
}

MdeModulePkg/Bus/Pci/NvmExpressDxe/UnitTest/MediaSanitizeUnitTestHost.inf  # MU_CHANGE - Add Additional Testing

# MU_CHANGE [BEGIN]
MdeModulePkg/Library/VariablePolicyLib/VariablePolicyUnitTest/VariablePolicyUnitTest.inf {
  <LibraryClasses>
    VariablePolicyLib|MdeModulePkg/Library/VariablePolicyLib/VariablePolicyLib.inf

  <PcdsFixedAtBuild>
    gEfiMdeModulePkgTokenSpaceGuid.PcdAllowVariablePolicyEnforcementDisable|TRUE
}
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
    BaseBinSecurityLib|MdePkg/Library/BaseBinSecurityLibNull/BaseBinSecurityLibNull.inf
    UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
    MemoryBinOverrideLib|MdeModulePkg/Library/MemoryBinOverrideLibNull/MemoryBinOverrideLibNull.inf
  <PcdsFixedAtBuild>
    gEfiMdeModulePkgTokenSpaceGuid.PcdLoadModuleAtFixAddressEnable|0
    gEfiMdeModulePkgTokenSpaceGuid.PcdLoadFixAddressRuntimeCodePageNumber|0
    gEfiMdeModulePkgTokenSpaceGuid.PcdLoadFixAddressBootTimeCodePageNumber|0
}
MdeModulePkg/Library/UefiSortLib/GoogleTest/UefiSortLibGoogleTest.inf {
  <LibraryClasses>
    SortLib|MdeModulePkg/Library/UefiSortLib/UefiSortLib.inf
    DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
}
MdeModulePkg/Test/Mock/Library/GoogleTest/MockPciHostBridgeLib/MockPciHostBridgeLib.inf
MdePkg/Test/UnitTest/Library/BaseSafeIntLib/TestBaseSafeIntLibHost.inf
MdePkg/Test/UnitTest/Library/BaseLib/BaseLibUnitTestsHost.inf
MdePkg/Test/GoogleTest/Library/BaseSafeIntLib/GoogleTestBaseSafeIntLib.inf
MdePkg/Test/Library/MockUefiBootServicesTableLib/MockUefiBootServicesTableLib.inf
MdePkg/Test/Library/MockUefiRuntimeServicesTableLib/MockUefiRuntimeServicesTableLib.inf
MdePkg/Test/Library/RngLibHostTestLfsr/RngLibHostTestLfsr.inf
MdePkg/Test/Library/StubHobLib/StubHobLib.inf
MdePkg/Test/Library/StubUefiLib/StubUefiLib.inf
MdePkg/Test/Library/SynchronizationLibHostUnitTest/SynchronizationLibHostUnitTest.inf
MdePkg/Library/BaseLib/UnitTestHostBaseLib.inf
MdePkg/Test/Mock/Library/GoogleTest/MockUefiLib/MockUefiLib.inf
MdePkg/Test/Mock/Library/GoogleTest/MockUefiRuntimeServicesTableLib/MockUefiRuntimeServicesTableLib.inf
MdePkg/Test/Mock/Library/GoogleTest/MockPeiServicesLib/MockPeiServicesLib.inf
MdePkg/Test/Mock/Library/GoogleTest/MockHobLib/MockHobLib.inf
PolicyServicePkg/PolicyService/DxeMm/UnitTest/DxeMmPolicyUnitTest.inf
PolicyServicePkg/PolicyService/Pei/UnitTest/PeiPolicyUnitTest.inf
UefiCpuPkg/Library/MtrrLib/UnitTest/MtrrLibUnitTestHost.inf
UefiCpuPkg/Library/CpuPageTableLib/UnitTest/CpuPageTableLibUnitTestHost.inf

[BuildOptions]
  *_*_*_CC_FLAGS            = -D DISABLE_NEW_DEPRECATED_INTERFACES