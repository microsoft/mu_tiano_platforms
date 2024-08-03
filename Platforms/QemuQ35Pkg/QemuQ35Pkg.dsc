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
  OUTPUT_DIRECTORY               = Build/QemuQ35Pkg
  SUPPORTED_ARCHITECTURES        = IA32|X64
  BUILD_TARGETS                  = NOOPT|DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT
  FLASH_DEFINITION               = QemuQ35Pkg/QemuQ35Pkg.fdf

  #
  # Defines for default states.  These can be changed on the command line.
  # -D FLAG=VALUE
  #
!ifndef DEBUGGER_ENABLED
  DEFINE DEBUGGER_ENABLED               = FALSE
!endif
!ifndef TPM_ENABLE
  DEFINE TPM_ENABLE                     = FALSE
!endif
  DEFINE TPM_CONFIG_ENABLE              = FALSE
  DEFINE OPT_INTO_MFCI_PRE_PRODUCTION   = TRUE
  DEFINE BUILD_UNIT_TESTS               = TRUE
  DEFINE PEI_MM_IPL_ENABLED             = TRUE
  DEFINE GUI_FRONT_PAGE                 = FALSE
  DEFINE TPM_REPLAY_ENABLED             = FALSE

  DEFINE NETWORK_HTTP_ENABLE            = TRUE
  DEFINE NETWORK_ALLOW_HTTP_CONNECTIONS = TRUE


  PEI_CRYPTO_SERVICES = TINY_SHA
  DXE_CRYPTO_SERVICES = STANDARD
  RUNTIMEDXE_CRYPTO_SERVICES = STANDARD
  SMM_CRYPTO_SERVICES = NONE
  STANDALONEMM_CRYPTO_SERVICES = STANDARD
  PEI_CRYPTO_ARCH = IA32
  DXE_CRYPTO_ARCH = X64
  RUNTIMEDXE_CRYPTO_ARCH = X64
  SMM_CRYPTO_ARCH = NONE
  STANDALONEMM_CRYPTO_ARCH = X64

################################################################################
#
# SKU Identification section - list of all SKU IDs supported by this Platform.
#
################################################################################
[SkuIds]
  0|DEFAULT

################################################################################
#
# Library Class section - list of all Library Classes needed by this Platform.
#
################################################################################
!include MdePkg/MdeLibs.dsc.inc

[LibraryClasses]
  PcdLib          |MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  TimerLib        |QemuQ35Pkg/Library/AcpiTimerLib/BaseAcpiTimerLib.inf
  PrintLib        |MdePkg/Library/BasePrintLib/BasePrintLib.inf
  BaseLib         |MdePkg/Library/BaseLib/BaseLib.inf
  UefiLib         |MdePkg/Library/UefiLib/UefiLib.inf
  HiiLib          |MdeModulePkg/Library/UefiHiiLib/UefiHiiLib.inf
  ResetSystemLib  |QemuQ35Pkg/Library/ResetSystemLib/BaseResetSystemLib.inf

  # Services tables/Entry points
  UefiBootServicesTableLib    |MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  UefiRuntimeServicesTableLib |MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  UefiDriverEntryPoint        |MdePkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.inf
  UefiApplicationEntryPoint   |MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
  DxeServicesLib              |MdePkg/Library/DxeServicesLib/DxeServicesLib.inf
  DxeServicesTableLib         |MdePkg/Library/DxeServicesTableLib/DxeServicesTableLib.inf
  UefiHiiServicesLib          |MdeModulePkg/Library/UefiHiiServicesLib/UefiHiiServicesLib.inf
  RegisterFilterLib           |MdePkg/Library/RegisterFilterLibNull/RegisterFilterLibNull.inf

  # Math Libraries
  FltUsedLib |MdePkg/Library/FltUsedLib/FltUsedLib.inf
  MathLib    |MsCorePkg/Library/MathLib/MathLib.inf
  SafeIntLib |MdePkg/Library/BaseSafeIntLib/BaseSafeIntLib.inf

  # PeCoff Libraries
  PeCoffLib              |MdePkg/Library/BasePeCoffLib/BasePeCoffLib.inf
  PeCoffGetEntryPointLib |MdePkg/Library/BasePeCoffGetEntryPointLib/BasePeCoffGetEntryPointLib.inf

  # Debug Libraries
  DebugPrintErrorLevelLib |MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
  OemHookStatusCodeLib    |MdeModulePkg/Library/OemHookStatusCodeLibNull/OemHookStatusCodeLibNull.inf
  PerformanceLib          |MdePkg/Library/BasePerformanceLibNull/BasePerformanceLibNull.inf
  ConsoleMsgLib           |PcBdsPkg/Library/ConsoleMsgLibNull/ConsoleMsgLibNull.inf
  PanicLib                |MdePkg/Library/BasePanicLibSerialPort/BasePanicLibSerialPort.inf
  PeCoffExtraActionLib    |MdePkg/Library/BasePeCoffExtraActionLibNull/BasePeCoffExtraActionLibNull.inf
  DebugAgentLib           |MdeModulePkg/Library/DebugAgentLibNull/DebugAgentLibNull.inf

  # Whea Libraries
  MsWheaEarlyStorageLib |MsWheaPkg/Library/MsWheaEarlyStorageLib/MsWheaEarlyStorageLib.inf
  MuTelemetryHelperLib  |MsWheaPkg/Library/MuTelemetryHelperLib/MuTelemetryHelperLib.inf

  # Boot and Boot Policy
  UefiBootManagerLib       |MdeModulePkg/Library/UefiBootManagerLib/UefiBootManagerLib.inf
  MsBootPolicyLib          |OemPkg/Library/MsBootPolicyLib/MsBootPolicyLib.inf
  DeviceBootManagerLib     |OemPkg/Library/DeviceBootManagerLib/DeviceBootManagerLib.inf
  MsAltBootLib             |OemPkg/Library/MsAltBootLib/MsAltBootLib.inf # interfaces with alternate boot variable
  MsBootOptionsLib         |QemuPkg/Library/MsBootOptionsLibQemu/MsBootOptionsLib.inf # attached to BdsDxe to implement Microsoft extensions to UefiBootManagerLib.
  MsBootManagerSettingsLib |OemPkg/Library/MsBootManagerSettingsDxeLib/MsBootManagerSettingsDxeLib.inf

  # UI and graphics
  BmpSupportLib            |MdeModulePkg/Library/BaseBmpSupportLib/BaseBmpSupportLib.inf
  BootLogoLib              |MdeModulePkg/Library/BootLogoLib/BootLogoLib.inf
  BootGraphicsProviderLib  |OemPkg/Library/BootGraphicsProviderLib/BootGraphicsProviderLib.inf #  uses PCDs and raw files in the firmware volumes to get Pcd
  CustomizedDisplayLib     |MdeModulePkg/Library/CustomizedDisplayLib/CustomizedDisplayLib.inf
  FrameBufferBltLib        |MdeModulePkg/Library/FrameBufferBltLib/FrameBufferBltLib.inf
  FrameBufferMemDrawLib    |MsGraphicsPkg/Library/FrameBufferMemDrawLib/FrameBufferMemDrawLibDxe.inf
  BootGraphicsLib          |MsGraphicsPkg/Library/BootGraphicsLib/BootGraphicsLib.inf
  GraphicsConsoleHelperLib |PcBdsPkg/Library/GraphicsConsoleHelperLib/GraphicsConsoleHelper.inf
  DisplayDeviceStateLib    |MsGraphicsPkg/Library/ColorBarDisplayDeviceStateLib/ColorBarDisplayDeviceStateLib.inf # Display the On screen notifications for the platform using color bars
  UIToolKitLib             |MsGraphicsPkg/Library/SimpleUIToolKit/SimpleUIToolKit.inf
  MsColorTableLib          |MsGraphicsPkg/Library/MsColorTableLib/MsColorTableLib.inf
  SwmDialogsLib            |MsGraphicsPkg/Library/SwmDialogsLib/SwmDialogs.inf #  Dialog Boxes in a Simple Window Manager environment.
  DeviceStateLib           |MdeModulePkg/Library/DeviceStateLib/DeviceStateLib.inf
  UiRectangleLib           |MsGraphicsPkg/Library/BaseUiRectangleLib/BaseUiRectangleLib.inf
  PlatformThemeLib         |QemuPkg/Library/PlatformThemeLib/PlatformThemeLib.inf # Q35 theme
  MsUiThemeCopyLib         |MsGraphicsPkg/Library/MsUiThemeCopyLib/MsUiThemeCopyLib.inf # handles copying the theme

  # CPU/SMBUS/Peripherals Libraries
  CpuLib              |MdePkg/Library/BaseCpuLib/BaseCpuLib.inf
  SynchronizationLib  |MdePkg/Library/BaseSynchronizationLib/BaseSynchronizationLib.inf
  LocalApicLib        |UefiCpuPkg/Library/BaseXApicX2ApicLib/BaseXApicX2ApicLib.inf
  SmbusLib            |MdePkg/Library/BaseSmbusLibNull/BaseSmbusLibNull.inf
  CacheMaintenanceLib |MdePkg/Library/BaseCacheMaintenanceLib/BaseCacheMaintenanceLib.inf
  MicrocodeLib        |UefiCpuPkg/Library/MicrocodeLib/MicrocodeLib.inf

  # File Libraries
  FileExplorerLib   |MdeModulePkg/Library/FileExplorerLib/FileExplorerLib.inf
  FileHandleLib     |MdePkg/Library/UefiFileHandleLib/UefiFileHandleLib.inf
  NvVarsFileLib     |QemuQ35Pkg/Library/NvVarsFileLib/NvVarsFileLib.inf
  UefiDecompressLib |MdePkg/Library/BaseUefiDecompressLib/BaseUefiDecompressLib.inf

  # Capsule/Versioning Libraries
  DisplayUpdateProgressLib |MdeModulePkg/Library/DisplayUpdateProgressLibText/DisplayUpdateProgressLibText.inf
  CapsulePersistLib |MdeModulePkg/Library/CapsulePersistLibNull/CapsulePersistLibNull.inf
  MuUefiVersionLib |OemPkg/Library/MuUefiVersionLib/MuUefiVersionLib.inf

  # Sorter helper Libraries
  SortLib              |MdeModulePkg/Library/UefiSortLib/UefiSortLib.inf
  OrderedCollectionLib |MdePkg/Library/BaseOrderedCollectionRedBlackTreeLib/BaseOrderedCollectionRedBlackTreeLib.inf

  # PCI libraries
  PciCf8Lib           |MdePkg/Library/BasePciCf8Lib/BasePciCf8Lib.inf
  PciExpressLib       |MdePkg/Library/BasePciExpressLib/BasePciExpressLib.inf
  PciLib              |MdePkg/Library/BasePciLibCf8/BasePciLibCf8.inf
  PciSegmentLib       |MdePkg/Library/BasePciSegmentLibPci/BasePciSegmentLibPci.inf
  PciCapLib           |QemuPkg/Library/BasePciCapLib/BasePciCapLib.inf
  PciCapPciSegmentLib |QemuPkg/Library/BasePciCapPciSegmentLib/BasePciCapPciSegmentLib.inf
  PciCapPciIoLib      |QemuPkg/Library/UefiPciCapPciIoLib/UefiPciCapPciIoLib.inf

  # IO Libraries
  IoLib         |MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsicSev.inf
  SerialPortLib |PcAtChipsetPkg/Library/SerialIoLib/SerialIoLib.inf
  VirtioLib     |QemuPkg/Library/VirtioLib/VirtioLib.inf
  TdxLib        |MdePkg/Library/TdxLib/TdxLib.inf
  CcProbeLib    |MdePkg/Library/CcProbeLibNull/CcProbeLibNull.inf

  # USB Libraries
  UefiUsbLib |MdePkg/Library/UefiUsbLib/UefiUsbLib.inf

  # Security Libraries
  SecurityManagementLib |MdeModulePkg/Library/DxeSecurityManagementLib/DxeSecurityManagementLib.inf
  SecurityLockAuditLib  |MdeModulePkg/Library/SecurityLockAuditDebugMessageLib/SecurityLockAuditDebugMessageLib.inf ##MU_CHANGE
  LockBoxLib            |QemuPkg/Library/LockBoxLib/LockBoxBaseLib.inf
  PlatformSecureLib     |SecurityPkg/Library/PlatformSecureLibNull/PlatformSecureLibNull.inf
  PasswordStoreLib      |MsCorePkg/Library/PasswordStoreLibNull/PasswordStoreLibNull.inf
  PasswordPolicyLib     |OemPkg/Library/PasswordPolicyLib/PasswordPolicyLib.inf
  SecureBootVariableLib |SecurityPkg/Library/SecureBootVariableLib/SecureBootVariableLib.inf
  SecureBootKeyStoreLib |MsCorePkg/Library/BaseSecureBootKeyStoreLib/BaseSecureBootKeyStoreLib.inf
  PlatformPKProtectionLib|SecurityPkg/Library/PlatformPKProtectionLibVarPolicy/PlatformPKProtectionLibVarPolicy.inf
  MuSecureBootKeySelectorLib|MsCorePkg/Library/MuSecureBootKeySelectorLib/MuSecureBootKeySelectorLib.inf

  # Memory Libraries
  BaseMemoryLib                  |MdePkg/Library/BaseMemoryLibRepStr/BaseMemoryLibRepStr.inf
  MemEncryptSevLib               |QemuQ35Pkg/Library/BaseMemEncryptSevLib/DxeMemEncryptSevLib.inf
  MemoryTypeInfoSecVarCheckLib   |MdeModulePkg/Library/MemoryTypeInfoSecVarCheckLib/MemoryTypeInfoSecVarCheckLib.inf # MU_CHANGE TCBZ1086
  MemoryTypeInformationChangeLib |MdeModulePkg/Library/MemoryTypeInformationChangeLibNull/MemoryTypeInformationChangeLibNull.inf
  MtrrLib                        |UefiCpuPkg/Library/MtrrLib/MtrrLib.inf # Memory Type Range Register (https://en.wikipedia.org/wiki/Memory_type_range_register)
  DxeMemoryProtectionHobLib      |MdeModulePkg/Library/MemoryProtectionHobLibNull/DxeMemoryProtectionHobLibNull.inf
  ExceptionPersistenceLib        |MsCorePkg/Library/ExceptionPersistenceLibCmos/ExceptionPersistenceLibCmos.inf
  CpuPageTableLib                |UefiCpuPkg/Library/CpuPageTableLib/CpuPageTableLib.inf
  FlatPageTableLib               |UefiTestingPkg/Library/FlatPageTableLib/FlatPageTableLib.inf
  ImagePropertiesRecordLib       |MdeModulePkg/Library/ImagePropertiesRecordLib/ImagePropertiesRecordLib.inf

  # Variable Libraries
  VariablePolicyLib         |MdeModulePkg/Library/VariablePolicyLib/VariablePolicyLib.inf
  VariablePolicyHelperLib   |MdeModulePkg/Library/VariablePolicyHelperLib/VariablePolicyHelperLib.inf
  AuthVariableLib           |SecurityPkg/Library/AuthVariableLib/AuthVariableLib.inf
  VarCheckLib               |MdeModulePkg/Library/VarCheckLib/VarCheckLib.inf
  SerializeVariablesLib     |QemuQ35Pkg/Library/SerializeVariablesLib/SerializeVariablesLib.inf
  VariableFlashInfoLib      |MdeModulePkg/Library/BaseVariableFlashInfoLib/BaseVariableFlashInfoLib.inf

  # Unit Test Libs
  UnitTestLib             |UnitTestFrameworkPkg/Library/UnitTestLib/UnitTestLib.inf
  UnitTestBootLib         |UnitTestFrameworkPkg/Library/UnitTestBootLibNull/UnitTestBootLibNull.inf
  UnitTestPersistenceLib  |UnitTestFrameworkPkg/Library/UnitTestPersistenceLibSimpleFileSystem/UnitTestPersistenceLibSimpleFileSystem.inf
  UnitTestResultReportLib |XmlSupportPkg/Library/UnitTestResultReportJUnitFormatLib/UnitTestResultReportLib.inf

  # Xen Libraries
  XenHypercallLib |QemuQ35Pkg/Library/XenHypercallLib/XenHypercallLib.inf
  XenPlatformLib  |QemuPkg/Library/XenPlatformLib/XenPlatformLib.inf

  # Crypto Libraries
  RngLib          |MdePkg/Library/BaseRngLib/BaseRngLib.inf
  Hash2CryptoLib  |SecurityPkg/Library/DxeHash2CryptoLib/DxeHash2CryptoLib.inf

  # Power/Thermal/Power State Libraries
  MsNVBootReasonLib       |OemPkg/Library/MsNVBootReasonLib/MsNVBootReasonLib.inf # interface on Reboot Reason non volatile variables
  ResetUtilityLib         |MdeModulePkg/Library/ResetUtilityLib/ResetUtilityLib.inf
  S3BootScriptLib         |MdeModulePkg/Library/PiDxeS3BootScriptLib/DxeS3BootScriptLib.inf
  PowerServicesLib        |PcBdsPkg/Library/PowerServicesLibNull/PowerServicesLibNull.inf
  ThermalServicesLib      |PcBdsPkg/Library/ThermalServicesLibNull/ThermalServicesLibNull.inf
  MsPlatformPowerCheckLib |PcBdsPkg/Library/MsPlatformPowerCheckLibNull/MsPlatformPowerCheckLibNull.inf

  # TPM Libraries
  OemTpm2InitLib          |SecurityPkg/Library/OemTpm2InitLibNull/OemTpm2InitLib.inf
  Tpm2DebugLib            |SecurityPkg/Library/Tpm2DebugLib/Tpm2DebugLibNull.inf
  Tpm12CommandLib         |SecurityPkg/Library/Tpm12CommandLib/Tpm12CommandLib.inf
  Tpm2CommandLib          |SecurityPkg/Library/Tpm2CommandLib/Tpm2CommandLib.inf
  Tpm2DeviceLib           |SecurityPkg/Library/Tpm2DeviceLibTcg2/Tpm2DeviceLibTcg2.inf
  Tcg2PpVendorLib         |SecurityPkg/Library/Tcg2PpVendorLibNull/Tcg2PpVendorLibNull.inf
  TpmMeasurementLib       |MdeModulePkg/Library/TpmMeasurementLibNull/TpmMeasurementLibNull.inf
  Tcg2PhysicalPresenceLib |QemuPkg/Library/Tcg2PhysicalPresenceLibNull/DxeTcg2PhysicalPresenceLib.inf
!if $(TPM_ENABLE) == TRUE
  Tcg2PhysicalPresenceLib |QemuPkg/Library/Tcg2PhysicalPresenceLibQemu/DxeTcg2PhysicalPresenceLib.inf
  TpmMeasurementLib       |SecurityPkg/Library/DxeTpmMeasurementLib/DxeTpmMeasurementLib.inf
  Tpm2DebugLib            |SecurityPkg/Library/Tpm2DebugLib/Tpm2DebugLibSimple.inf
!endif

  # Shell Libraries
  ShellLib         |ShellPkg/Library/UefiShellLib/UefiShellLib.inf
  ShellCommandLib  |ShellPkg/Library/UefiShellCommandLib/UefiShellCommandLib.inf
  ShellCEntryLib   |ShellPkg/Library/UefiShellCEntryLib/UefiShellCEntryLib.inf
  HandleParsingLib |ShellPkg/Library/UefiHandleParsingLib/UefiHandleParsingLib.inf
  BcfgCommandLib   |ShellPkg/Library/UefiShellBcfgCommandLib/UefiShellBcfgCommandLib.inf

  # DFCI / XML / JSON Libraries
  DfciUiSupportLib                  |QemuPkg/Library/DfciUiSupportLib/DfciUiSupportLib.inf # Supports DFCI Groups.
  DfciV1SupportLib                  |DfciPkg/Library/DfciV1SupportLibNull/DfciV1SupportLibNull.inf # Backwards compatibility with DFCI V1 functions.
  DfciDeviceIdSupportLib            |QemuPkg/Library/DfciDeviceIdSupportLib/DfciDeviceIdSupportLib.inf
  DfciGroupLib                      |DfciPkg/Library/DfciGroupLibNull/DfciGroups.inf
  DfciRecoveryLib                   |DfciPkg/Library/DfciRecoveryLib/DfciRecoveryLib.inf
  SwmDialogsLib                     |MsGraphicsPkg/Library/SwmDialogsLib/SwmDialogs.inf
   # Zero Touch is part of DFCI
  ZeroTouchSettingsLib              |ZeroTouchPkg/Library/ZeroTouchSettings/ZeroTouchSettings.inf
   # Libraries that understands the MsXml Settings Schema and providers helper functions
  DfciXmlIdentitySchemaSupportLib   |DfciPkg/Library/DfciXmlIdentitySchemaSupportLib/DfciXmlIdentitySchemaSupportLib.inf
  DfciXmlDeviceIdSchemaSupportLib   |DfciPkg/Library/DfciXmlDeviceIdSchemaSupportLib/DfciXmlDeviceIdSchemaSupportLib.inf
  DfciXmlSettingSchemaSupportLib    |DfciPkg/Library/DfciXmlSettingSchemaSupportLib/DfciXmlSettingSchemaSupportLib.inf
  DfciXmlPermissionSchemaSupportLib |DfciPkg/Library/DfciXmlPermissionSchemaSupportLib/DfciXmlPermissionSchemaSupportLib.inf
  DfciSettingChangedNotificationLib |DfciPkg/Library/DfciSettingChangedNotificationLib/DfciSettingChangedNotificationLibNull.inf

   #XML libraries
  XmlTreeQueryLib                   |XmlSupportPkg/Library/XmlTreeQueryLib/XmlTreeQueryLib.inf
  XmlTreeLib                        |XmlSupportPkg/Library/XmlTreeLib/XmlTreeLib.inf
   # Json parser
  JsonLiteParserLib |MsCorePkg/Library/JsonLiteParser/JsonLiteParser.inf

  # Qemu specific libraries
  QemuFwCfgLib             |QemuQ35Pkg/Library/QemuFwCfgLib/QemuFwCfgDxeLib.inf
  QemuFwCfgSimpleParserLib |QemuQ35Pkg/Library/QemuFwCfgSimpleParserLib/QemuFwCfgSimpleParserLib.inf
  CcExitLib                |UefiCpuPkg/Library/CcExitLibNull/CcExitLibNull.inf

  # Platform devices path libraries
  MsPlatformDevicesLib |QemuQ35Pkg/Library/MsPlatformDevicesLibQemuQ35/MsPlatformDevicesLib.inf
  DevicePathLib        |MdePkg/Library/UefiDevicePathLibDevicePathProtocol/UefiDevicePathLibDevicePathProtocol.inf
  LoadLinuxLib         |QemuQ35Pkg/Library/LoadLinuxLib/LoadLinuxLib.inf

  # Setup variable libraries
  SvdXmlSettingSchemaSupportLib |SetupDataPkg/Library/SvdXmlSettingSchemaSupportLib/SvdXmlSettingSchemaSupportLib.inf
  ConfigVariableListLib         |SetupDataPkg/Library/ConfigVariableListLib/ConfigVariableListLib.inf
  ConfigSystemModeLib           |QemuPkg/Library/ConfigSystemModeLibQemu/ConfigSystemModeLib.inf
  ActiveProfileIndexSelectorLib |OemPkg/Library/ActiveProfileIndexSelectorPcdLib/ActiveProfileIndexSelectorPcdLib.inf

  # Network libraries
  NetLib                 |NetworkPkg/Library/DxeNetLib/DxeNetLib.inf
  MsNetworkDependencyLib |PcBdsPkg/Library/MsNetworkDependencyLib/MsNetworkDependencyLib.inf # Library that is attached to drivers that require networking.
  !include NetworkPkg/NetworkLibs.dsc.inc

  PlatformSmmProtectionsTestLib|UefiTestingPkg/Library/PlatformSmmProtectionsTestLibNull/PlatformSmmProtectionsTestLibNull.inf
  FmpDependencyLib|FmpDevicePkg/Library/FmpDependencyLib/FmpDependencyLib.inf

  # Debugger Libraries
  DebugTransportLib|QemuQ35Pkg/Library/DebugTransportSerialIoWrapperLib/DebugTransportSerialIoWrapperLib.inf
  WatchdogTimerLib|DebuggerFeaturePkg/Library/WatchdogTimerLibNull/WatchdogTimerLibNull.inf
  TransportLogControlLib|DebuggerFeaturePkg/Library/TransportLogControlLibNull/TransportLogControlLibNull.inf

[LibraryClasses]
  # Platform Runtime Mechanism (PRM) libraries
  PrmContextBufferLib|PrmPkg/Library/DxePrmContextBufferLib/DxePrmContextBufferLib.inf
  PrmModuleDiscoveryLib|PrmPkg/Library/DxePrmModuleDiscoveryLib/DxePrmModuleDiscoveryLib.inf
  PrmPeCoffLib|PrmPkg/Library/DxePrmPeCoffLib/DxePrmPeCoffLib.inf

  StackCheckFailureHookLib|MdePkg/Library/StackCheckFailureHookLibNull/StackCheckFailureHookLibNull.inf
[LibraryClasses.common.PEI_CORE]
  NULL|MdePkg/Library/StackCheckLibNull/StackCheckLibNull.inf

[LibraryClasses.common.PEIM]
  NULL|MdePkg/Library/StackCheckLib/StackCheckLibStaticInit.inf

[LibraryClasses.common.DXE_CORE, LibraryClasses.common.SMM_CORE, LibraryClasses.common.DXE_SMM_DRIVER, LibraryClasses.common.DXE_DRIVER, LibraryClasses.common.DXE_RUNTIME_DRIVER, LibraryClasses.common.DXE_SAL_DRIVER, LibraryClasses.common.UEFI_DRIVER, LibraryClasses.common.UEFI_APPLICATION, LibraryClasses.common.MM_CORE_STANDALONE, LibraryClasses.common.MM_STANDALONE]
  NULL|MdePkg/Library/StackCheckLib/StackCheckLibDynamicInit.inf

#########################################
# SEC Libraries
#########################################
[LibraryClasses.common.SEC]
  TimerLib                   |QemuQ35Pkg/Library/AcpiTimerLib/BaseRomAcpiTimerLib.inf
  QemuFwCfgLib               |QemuQ35Pkg/Library/QemuFwCfgLib/QemuFwCfgSecLib.inf
  HobLib                     |MdePkg/Library/PeiHobLib/PeiHobLib.inf
  PeiServicesLib             |MdePkg/Library/PeiServicesLib/PeiServicesLib.inf
  PeiServicesTablePointerLib |MdePkg/Library/PeiServicesTablePointerLibIdt/PeiServicesTablePointerLibIdt.inf
  MemoryAllocationLib        |MdePkg/Library/PeiMemoryAllocationLib/PeiMemoryAllocationLib.inf
  CpuExceptionHandlerLib     |UefiCpuPkg/Library/CpuExceptionHandlerLib/SecPeiCpuExceptionHandlerLib.inf
  ReportStatusCodeLib        |MdeModulePkg/Library/PeiReportStatusCodeLib/PeiReportStatusCodeLib.inf
  ExtractGuidedSectionLib    |MdePkg/Library/BaseExtractGuidedSectionLib/BaseExtractGuidedSectionLib.inf

!ifndef $(DEBUG_ON_SERIAL_PORT)
  DebugLib                   |QemuQ35Pkg/Library/PlatformDebugLibIoPort/PlatformRomDebugLibIoPort.inf
!endif

  MemEncryptSevLib|QemuQ35Pkg/Library/BaseMemEncryptSevLib/SecMemEncryptSevLib.inf

[LibraryClasses.common.DXE_DRIVER, LibraryClasses.common.DXE_CORE, LibraryClasses.common.UEFI_APPLICATION]
  DxeMemoryProtectionHobLib|MdeModulePkg/Library/MemoryProtectionHobLib/DxeMemoryProtectionHobLib.inf

[LibraryClasses.common.SMM_CORE, LibraryClasses.common.DXE_SMM_DRIVER]
  MmMemoryProtectionHobLib|MdeModulePkg/Library/MemoryProtectionHobLib/SmmMemoryProtectionHobLib.inf

[LibraryClasses.common.MM_CORE_STANDALONE, LibraryClasses.common.MM_STANDALONE]
  MmMemoryProtectionHobLib|MdeModulePkg/Library/MemoryProtectionHobLib/StandaloneMmMemoryProtectionHobLib.inf

#########################################
# PEI Libraries
#########################################
[LibraryClasses.common.PEI_CORE, LibraryClasses.common.PEIM]
  HobLib                     |MdePkg/Library/PeiHobLib/PeiHobLib.inf
  PeiServicesTablePointerLib |MdePkg/Library/PeiServicesTablePointerLibIdt/PeiServicesTablePointerLibIdt.inf
  PeiServicesLib             |MdePkg/Library/PeiServicesLib/PeiServicesLib.inf
  MemoryAllocationLib        |MdePkg/Library/PeiMemoryAllocationLib/PeiMemoryAllocationLib.inf
  ReportStatusCodeLib        |MdeModulePkg/Library/PeiReportStatusCodeLib/PeiReportStatusCodeLib.inf
  MemEncryptSevLib           |QemuQ35Pkg/Library/BaseMemEncryptSevLib/PeiMemEncryptSevLib.inf
  FrameBufferMemDrawLib      |MsGraphicsPkg/Library/FrameBufferMemDrawLib/FrameBufferMemDrawLibPei.inf
  MmUnblockMemoryLib         |MmSupervisorPkg/Library/MmSupervisorUnblockMemoryLib/MmSupervisorUnblockMemoryLibPei.inf

[LibraryClasses.common.PEI_CORE]
  PeiCoreEntryPoint |MdePkg/Library/PeiCoreEntryPoint/PeiCoreEntryPoint.inf
  RngLib|MdeModulePkg/Library/BaseRngLibTimerLib/BaseRngLibTimerLib.inf

[LibraryClasses.common.PEIM]
  ResetSystemLib             |MdeModulePkg/Library/PeiResetSystemLib/PeiResetSystemLib.inf
  HwResetSystemLib           |QemuQ35Pkg/Library/ResetSystemLib/BaseResetSystemLib.inf
  PeimEntryPoint             |MdePkg/Library/PeimEntryPoint/PeimEntryPoint.inf
  MsPlatformEarlyGraphicsLib |MsGraphicsPkg/Library/MsEarlyGraphicsLibNull/Pei/MsEarlyGraphicsLibNull.inf
  MsUiThemeLib               |MsGraphicsPkg/Library/MsUiThemeLib/Pei/MsUiThemeLib.inf
  LockBoxLib                 |MdeModulePkg/Library/SmmLockBoxLib/SmmLockBoxPeiLib.inf
  ResourcePublicationLib     |MdePkg/Library/PeiResourcePublicationLib/PeiResourcePublicationLib.inf
  ExtractGuidedSectionLib    |MdePkg/Library/PeiExtractGuidedSectionLib/PeiExtractGuidedSectionLib.inf
  CpuExceptionHandlerLib     |UefiCpuPkg/Library/CpuExceptionHandlerLib/PeiCpuExceptionHandlerLib.inf
  MpInitLib                  |UefiCpuPkg/Library/MpInitLib/PeiMpInitLib.inf
  QemuFwCfgS3Lib             |QemuQ35Pkg/Library/QemuFwCfgS3Lib/PeiQemuFwCfgS3LibFwCfg.inf
  PcdLib                     |MdePkg/Library/PeiPcdLib/PeiPcdLib.inf
  QemuFwCfgLib               |QemuQ35Pkg/Library/QemuFwCfgLib/QemuFwCfgPeiLib.inf
  PcdDatabaseLoaderLib       |MdeModulePkg/Library/PcdDatabaseLoaderLib/Pei/PcdDatabaseLoaderLibPei.inf
  OemMfciLib                 |OemPkg/Library/OemMfciLib/OemMfciLibPei.inf
  ConfigKnobShimLib          |SetupDataPkg/Library/ConfigKnobShimLib/ConfigKnobShimPeiLib/ConfigKnobShimPeiLib.inf
  PolicyLib                  |PolicyServicePkg/Library/PeiPolicyLib/PeiPolicyLib.inf
!if $(TPM_ENABLE) == TRUE
  Tpm12DeviceLib             |SecurityPkg/Library/Tpm12DeviceLibDTpm/Tpm12DeviceLibDTpm.inf
  Tpm2DeviceLib              |SecurityPkg/Library/Tpm2DeviceLibDTpm/Tpm2DeviceLibDTpm.inf
  SourceDebugEnabledLib      |SourceLevelDebugPkg/Library/SourceDebugEnabled/SourceDebugEnabledLib.inf
  Tcg2PreUefiEventLogLib     |QemuPkg/Library/QemuPreUefiEventLogLibNull/QemuPreUefiEventLogLibNull.inf
!endif
  RngLib                     |MdePkg/Library/PeiRngLib/PeiRngLib.inf

[LibraryClasses.X64.PEIM]
  CpuExceptionHandlerLib     |UefiCpuPkg/Library/CpuExceptionHandlerLib/SecPeiCpuExceptionHandlerLib.inf

#########################################
# DXE Libraries
#########################################
# Common to all DXE phases
[LibraryClasses.common.DXE_CORE, LibraryClasses.common.DXE_RUNTIME_DRIVER, LibraryClasses.common.UEFI_DRIVER, LibraryClasses.common.DXE_DRIVER, LibraryClasses.common.UEFI_APPLICATION]
  MsUiThemeLib                  |MsGraphicsPkg/Library/MsUiThemeLib/Dxe/MsUiThemeLib.inf
  MsPlatformEarlyGraphicsLib    |MsGraphicsPkg/Library/MsEarlyGraphicsLibNull/Dxe/MsEarlyGraphicsLibNull.inf
  PcdLib                        |MdePkg/Library/DxePcdLib/DxePcdLib.inf
  HobLib                        |MdePkg/Library/DxeHobLib/DxeHobLib.inf
  ResetSystemLib                |MdeModulePkg/Library/DxeResetSystemLib/DxeResetSystemLib.inf
  HwResetSystemLib              |QemuQ35Pkg/Library/ResetSystemLib/BaseResetSystemLib.inf
  DxeCoreEntryPoint             |MdePkg/Library/DxeCoreEntryPoint/DxeCoreEntryPoint.inf
  MemoryAllocationLib           |MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  CpuExceptionHandlerLib        |UefiCpuPkg/Library/CpuExceptionHandlerLib/DxeCpuExceptionHandlerLib.inf
  ReportStatusCodeLib           |MdeModulePkg/Library/DxeReportStatusCodeLib/DxeReportStatusCodeLib.inf
  QemuFwCfgS3Lib                |QemuQ35Pkg/Library/QemuFwCfgS3Lib/DxeQemuFwCfgS3LibFwCfg.inf
  MmUnblockMemoryLib            |MmSupervisorPkg/Library/MmSupervisorUnblockMemoryLib/MmSupervisorUnblockMemoryLibDxe.inf

# Non DXE Core but everything else
[LibraryClasses.common.DXE_RUNTIME_DRIVER, LibraryClasses.common.UEFI_DRIVER, LibraryClasses.common.DXE_DRIVER, LibraryClasses.common.UEFI_APPLICATION]
  TimerLib |QemuQ35Pkg/Library/AcpiTimerLib/DxeAcpiTimerLib.inf
  RngLib   |MdePkg/Library/DxeRngLib/DxeRngLib.inf
  PciLib   |QemuQ35Pkg/Library/DxePciLibI440FxQ35/DxePciLibI440FxQ35.inf

  OemMfciLib |OemPkg/Library/OemMfciLib/OemMfciLibDxe.inf

[LibraryClasses.common.DXE_CORE]
  HobLib                  |MdePkg/Library/DxeCoreHobLib/DxeCoreHobLib.inf
  MemoryAllocationLib     |MdeModulePkg/Library/DxeCoreMemoryAllocationLib/DxeCoreMemoryAllocationLib.inf
  ExtractGuidedSectionLib |MdePkg/Library/DxeExtractGuidedSectionLib/DxeExtractGuidedSectionLib.inf
  DebugAgentLib           |DebuggerFeaturePkg/Library/DebugAgent/DebugAgentDxe.inf
  MemoryBinOverrideLib    |MdeModulePkg/Library/MemoryBinOverrideLibNull/MemoryBinOverrideLibNull.inf
  RngLib                  |MdeModulePkg/Library/BaseRngLibTimerLib/BaseRngLibTimerLib.inf


[LibraryClasses.common.DXE_RUNTIME_DRIVER]
  ReportStatusCodeLib|MdeModulePkg/Library/RuntimeDxeReportStatusCodeLib/RuntimeDxeReportStatusCodeLib.inf
  ResetSystemLib|QemuQ35Pkg/Library/ResetSystemLib/DxeResetSystemLib.inf
  HwResetSystemLib|QemuQ35Pkg/Library/ResetSystemLib/DxeResetSystemLib.inf
  UefiRuntimeLib|MdePkg/Library/UefiRuntimeLib/UefiRuntimeLib.inf
  CapsuleLib|MdeModulePkg/Library/DxeCapsuleLibFmp/DxeRuntimeCapsuleLib.inf
  VariablePolicyLib|MdeModulePkg/Library/VariablePolicyLib/VariablePolicyLibRuntimeDxe.inf

[LibraryClasses.common.UEFI_DRIVER, LibraryClasses.common.DXE_DRIVER]
  UefiScsiLib|MdePkg/Library/UefiScsiLib/UefiScsiLib.inf

[LibraryClasses.common.UEFI_APPLICATION]
  CheckHwErrRecHeaderLib|MsWheaPkg/Library/CheckHwErrRecHeaderLib/CheckHwErrRecHeaderLib.inf
  FlatPageTableLib|UefiTestingPkg/Library/FlatPageTableLib/FlatPageTableLib.inf

[LibraryClasses.common.DXE_DRIVER]
  PlatformBootManagerLib|MsCorePkg/Library/PlatformBootManagerLib/PlatformBootManagerLib.inf
  PlatformBmPrintScLib|QemuPkg/Library/PlatformBmPrintScLib/PlatformBmPrintScLib.inf
  LockBoxLib|MdeModulePkg/Library/SmmLockBoxLib/SmmLockBoxDxeLib.inf
  QemuLoadImageLib|QemuQ35Pkg/Library/GenericQemuLoadImageLib/GenericQemuLoadImageLib.inf
  MpInitLib|UefiCpuPkg/Library/MpInitLib/DxeMpInitLib.inf
  UpdateFacsHardwareSignatureLib|OemPkg/Library/UpdateFacsHardwareSignatureLib/UpdateFacsHardwareSignatureLib.inf
  PcdDatabaseLoaderLib|MdeModulePkg/Library/PcdDatabaseLoaderLib/Dxe/PcdDatabaseLoaderLibDxe.inf
  CapsuleLib|MdeModulePkg/Library/DxeCapsuleLibFmp/DxeCapsuleLib.inf
  PolicyLib|PolicyServicePkg/Library/DxePolicyLib/DxePolicyLib.inf
!if $(TPM_ENABLE) == TRUE
  Tpm12DeviceLib|SecurityPkg/Library/Tpm12DeviceLibTcg/Tpm12DeviceLibTcg.inf
  Tpm2DeviceLib|SecurityPkg/Library/Tpm2DeviceLibTcg2/Tpm2DeviceLibTcg2.inf
!endif

#########################################
# SMM Libraries
#########################################
[LibraryClasses.common.SMM_CORE, LibraryClasses.common.DXE_SMM_DRIVER]
  ResetSystemLib|QemuQ35Pkg/Library/ResetSystemLib/DxeResetSystemLib.inf
  HwResetSystemLib|QemuQ35Pkg/Library/ResetSystemLib/DxeResetSystemLib.inf
  ReportStatusCodeLib|MdeModulePkg/Library/SmmReportStatusCodeLib/SmmReportStatusCodeLib.inf
  HobLib|MdePkg/Library/DxeHobLib/DxeHobLib.inf
  PciLib|QemuQ35Pkg/Library/DxePciLibI440FxQ35/DxePciLibI440FxQ35.inf
  PcdLib|MdePkg/Library/DxePcdLib/DxePcdLib.inf
  TimerLib|QemuQ35Pkg/Library/AcpiTimerLib/DxeAcpiTimerLib.inf

[LibraryClasses.common.DXE_SMM_DRIVER]
  MemoryAllocationLib|MdePkg/Library/SmmMemoryAllocationLib/SmmMemoryAllocationLib.inf
  SmmMemLib|MdePkg/Library/SmmMemLib/SmmMemLib.inf
  MmServicesTableLib|MdePkg/Library/MmServicesTableLib/MmServicesTableLib.inf
  SmmServicesTableLib|MdePkg/Library/SmmServicesTableLib/SmmServicesTableLib.inf
  CpuExceptionHandlerLib|UefiCpuPkg/Library/CpuExceptionHandlerLib/SmmCpuExceptionHandlerLib.inf
  RngLib|MdeModulePkg/Library/BaseRngLibTimerLib/BaseRngLibTimerLib.inf
  LockBoxLib|MdeModulePkg/Library/SmmLockBoxLib/SmmLockBoxSmmLib.inf
  AdvLoggerAccessLib|AdvLoggerPkg/Library/AdvLoggerSmmAccessLib/AdvLoggerSmmAccessLib.inf

[LibraryClasses.common.SMM_CORE]
  SmmCorePlatformHookLib|MdeModulePkg/Library/SmmCorePlatformHookLibNull/SmmCorePlatformHookLibNull.inf
  MemoryAllocationLib|MdeModulePkg/Library/PiSmmCoreMemoryAllocationLib/PiSmmCoreMemoryAllocationLib.inf
  SmmMemLib|MdePkg/Library/SmmMemLib/SmmMemLib.inf
  SmmServicesTableLib|MdeModulePkg/Library/PiSmmCoreSmmServicesTableLib/PiSmmCoreSmmServicesTableLib.inf

[LibraryClasses.common.MM_CORE_STANDALONE]
  TimerLib|QemuQ35Pkg/Library/AcpiTimerLib/DxeAcpiTimerLib.inf
  ExtractGuidedSectionLib|MdePkg/Library/BaseExtractGuidedSectionLib/BaseExtractGuidedSectionLib.inf
  FvLib|StandaloneMmPkg/Library/FvLib/FvLib.inf
  HobLib|StandaloneMmPkg/Library/StandaloneMmCoreHobLib/StandaloneMmCoreHobLib.inf
  MemoryAllocationLib|StandaloneMmPkg/Library/StandaloneMmCoreMemoryAllocationLib/StandaloneMmCoreMemoryAllocationLib.inf
  MemLib|MmSupervisorPkg/Library/MmSupervisorMemLib/MmSupervisorCoreMemLib.inf
  ReportStatusCodeLib|MdePkg/Library/BaseReportStatusCodeLibNull/BaseReportStatusCodeLibNull.inf
  StandaloneMmCoreEntryPoint|StandaloneMmPkg/Library/StandaloneMmCoreEntryPoint/StandaloneMmCoreEntryPoint.inf
  SmmCpuFeaturesLib|QemuQ35Pkg/Library/SmmCpuFeaturesLib/StandaloneMmCpuFeaturesLib.inf
  SmmCpuPlatformHookLib|QemuQ35Pkg/Library/SmmCpuPlatformHookLibQemu/SmmCpuPlatformHookLibQemu.inf
  CpuExceptionHandlerLib|UefiCpuPkg/Library/CpuExceptionHandlerLib/SmmCpuExceptionHandlerLib.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLibStandaloneMm.inf
  SortLib|MdeModulePkg/Library/BaseSortLib/BaseSortLib.inf
  SmmPolicyGateLib|MmSupervisorPkg/Library/SmmPolicyGateLib/SmmPolicyGateLib.inf
  HwResetSystemLib|QemuQ35Pkg/Library/ResetSystemLib/StandaloneMmResetSystemLib.inf
  IhvSmmSaveStateSupervisionLib|MmSupervisorPkg/Library/IhvMmSaveStateSupervisionLib/IhvMmSaveStateSupervisionLib.inf
  MmServicesTableLib|StandaloneMmPkg/Library/StandaloneMmServicesTableLib/StandaloneMmServicesTableLibCore.inf
  MmSaveStateLib|UefiCpuPkg/Library/MmSaveStateLib/AmdMmSaveStateLib.inf
  SmmCpuSyncLib|MmSupervisorPkg/Library/StandaloneMmCpuSyncLib/StandaloneMmCpuSyncLib.inf
  RngLib|MdeModulePkg/Library/BaseRngLibTimerLib/BaseRngLibTimerLib.inf

[LibraryClasses.common.MM_STANDALONE]
  TimerLib|QemuQ35Pkg/Library/AcpiTimerLib/DxeAcpiTimerLib.inf
  MmServicesTableLib|MmSupervisorPkg/Library/StandaloneMmServicesTableLib/StandaloneMmServicesTableLib.inf
  MemoryAllocationLib|StandaloneMmPkg/Library/StandaloneMmMemoryAllocationLib/StandaloneMmMemoryAllocationLib.inf
  HobLib|MmSupervisorPkg/Library/StandaloneMmHobLibSyscall/StandaloneMmHobLibSyscall.inf
  MemLib|MmSupervisorPkg/Library/MmSupervisorMemLib/MmSupervisorMemLibSyscall.inf
  LockBoxLib|MdeModulePkg/Library/SmmLockBoxLib/SmmLockBoxStandaloneMmLib.inf
  CpuExceptionHandlerLib|UefiCpuPkg/Library/CpuExceptionHandlerLib/SmmCpuExceptionHandlerLib.inf
  ReportStatusCodeLib|MdeModulePkg/Library/SmmReportStatusCodeLib/StandaloneMmReportStatusCodeLib.inf
  HwResetSystemLib|QemuQ35Pkg/Library/ResetSystemLib/StandaloneMmResetSystemLib.inf
  StandaloneMmDriverEntryPoint|MmSupervisorPkg/Library/StandaloneMmDriverEntryPoint/StandaloneMmDriverEntryPoint.inf
  AdvLoggerAccessLib|AdvLoggerPkg/Library/AdvLoggerMmAccessLib/AdvLoggerMmAccessLib.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLibStandaloneMm.inf
  RngLib|MdeModulePkg/Library/BaseRngLibTimerLib/BaseRngLibTimerLib.inf
  PciLib|QemuQ35Pkg/Library/DxePciLibI440FxQ35/DxePciLibI440FxQ35.inf

  BaseLib|MmSupervisorPkg/Library/BaseLibSysCall/BaseLib.inf
  IoLib|MmSupervisorPkg/Library/BaseIoLibIntrinsicSysCall/BaseIoLibIntrinsic.inf
  SysCallLib|MmSupervisorPkg/Library/SysCallLib/SysCallLib.inf
  CpuLib|MmSupervisorPkg/Library/BaseCpuLibSysCall/BaseCpuLib.inf

#########################################
# Advanced Logger Libraries
#########################################
[LibraryClasses]
!ifndef $(DEBUG_ON_SERIAL_PORT)
  DebugLib|AdvLoggerPkg/Library/BaseDebugLibAdvancedLogger/BaseDebugLibAdvancedLogger.inf
!else
  DebugLib|MdePkg/Library/BaseDebugLibSerialPort/BaseDebugLibSerialPort.inf
!endif
  AdvancedLoggerHdwPortLib|AdvLoggerPkg/Library/AdvancedLoggerHdwPortLib/AdvancedLoggerHdwPortLib.inf
  AssertLib|AdvLoggerPkg/Library/AssertLib/AssertLib.inf
  AdvancedLoggerAccessLib|AdvLoggerPkg/Library/AdvancedLoggerAccessLib/AdvancedLoggerAccessLib.inf

[LibraryClasses.IA32.PEI_CORE]
  AdvancedLoggerLib|AdvLoggerPkg/Library/AdvancedLoggerLib/PeiCore/AdvancedLoggerLib.inf

[LibraryClasses.IA32.PEIM]
  AdvancedLoggerLib|AdvLoggerPkg/Library/AdvancedLoggerLib/Pei/AdvancedLoggerLib.inf
!ifndef $(DEBUG_ON_SERIAL_PORT)
  DebugLib|AdvLoggerPkg/Library/PeiDebugLibAdvancedLogger/PeiDebugLibAdvancedLogger.inf
!endif

[LibraryClasses.X64.PEIM]
  AdvancedLoggerLib|AdvLoggerPkg/Library/AdvancedLoggerLib/Pei64/AdvancedLoggerLib.inf

[LibraryClasses.X64]
  AdvancedLoggerLib|AdvLoggerPkg/Library/AdvancedLoggerLib/Dxe/AdvancedLoggerLib.inf
  AdvancedLoggerAccessLib|AdvLoggerPkg/Library/AdvancedLoggerAccessLib/AdvancedLoggerAccessLib.inf

[LibraryClasses.X64.DXE_CORE]
  AdvancedLoggerLib|AdvLoggerPkg/Library/AdvancedLoggerLib/DxeCore/AdvancedLoggerLib.inf

[LibraryClasses.X64.DXE_SMM_DRIVER]
  AdvancedLoggerLib|AdvLoggerPkg/Library/AdvancedLoggerLib/Smm/AdvancedLoggerLib.inf

[LibraryClasses.X64.SMM_CORE]
  AdvancedLoggerLib|AdvLoggerPkg/Library/AdvancedLoggerLib/Smm/AdvancedLoggerLib.inf

[LibraryClasses.X64.DXE_RUNTIME_DRIVER]
  AdvancedLoggerLib|AdvLoggerPkg/Library/AdvancedLoggerLib/Runtime/AdvancedLoggerLib.inf

[LibraryClasses.X64.MM_CORE_STANDALONE, LibraryClasses.X64.MM_STANDALONE]
  AdvancedLoggerLib|AdvLoggerPkg/Library/AdvancedLoggerLib/MmCore/AdvancedLoggerLib.inf

################################################################################
#
# Pcd Section - list of all EDK II PCD Entries defined by this Platform.
#
################################################################################
[PcdsFeatureFlag]
  gEfiMdeModulePkgTokenSpaceGuid.PcdHiiOsRuntimeSupport|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdDxeIplSupportUefiDecompress|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdDxeIplSwitchToLongMode|TRUE
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutGopSupport|TRUE
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutUgaSupport|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdInstallAcpiSdtProtocol|TRUE
  gEmbeddedTokenSpaceGuid.PcdPrePiProduceMemoryTypeInformationHob|TRUE
  gAdvLoggerPkgTokenSpaceGuid.PcdAdvancedLoggerLocator|TRUE
  gAdvLoggerPkgTokenSpaceGuid.PcdAdvancedLoggerAutoWrapEnable|TRUE

  gQemuPkgTokenSpaceGuid.PcdSmmSmramRequire|$(SMM_ENABLED)
  gUefiQemuQ35PkgTokenSpaceGuid.PcdStandaloneMmEnable|$(SMM_ENABLED)
  gUefiCpuPkgTokenSpaceGuid.PcdCpuHotPlugSupport|FALSE

  gEfiMdeModulePkgTokenSpaceGuid.PcdRequireIommu|FALSE # don't require IOMMU
  gQemuPkgTokenSpaceGuid.PcdEnableMemoryProtection|$(MEMORY_PROTECTION)

  !if $(BUILD_UNIT_TESTS) == TRUE
    gUefiCpuPkgTokenSpaceGuid.PcdSmmExceptionTestModeSupport|TRUE
    gMmSupervisorPkgTokenSpaceGuid.PcdMmSupervisorTestEnable|TRUE
  !endif

[PcdsPatchableInModule]
!if $(DEBUGGER_ENABLED) == TRUE
  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x17
!else
  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x2F
!endif

[PcdsFixedAtBuild]
  !include QemuPkg/AutoGen/SecurebootPcds.inc
  gEfiMdeModulePkgTokenSpaceGuid.PcdStatusCodeMemorySize|1
  gEfiMdeModulePkgTokenSpaceGuid.PcdResetOnMemoryTypeInformationChange|TRUE
  gEfiMdePkgTokenSpaceGuid.PcdMaximumGuidedExtractHandler|0x10
  gEfiMdeModulePkgTokenSpaceGuid.PcdMaxVariableSize|0x8400
  gEfiMdeModulePkgTokenSpaceGuid.PcdMaxAuthVariableSize|0x8400
  gEfiMdeModulePkgTokenSpaceGuid.PcdHwErrStorageSize|0x1000
  gPcBdsPkgTokenSpaceGuid.PcdEnableMemMapOutput|0x1
  gAdvLoggerPkgTokenSpaceGuid.PcdAdvancedFileLoggerFlush|3
  gAdvLoggerPkgTokenSpaceGuid.PcdAdvancedLoggerPreMemPages|3
  gEfiSecurityPkgTokenSpaceGuid.PcdUserPhysicalPresence|FALSE

!if $(NETWORK_TLS_ENABLE) == FALSE
  # match PcdFlashNvStorageVariableSize purely for convenience
  gEfiMdeModulePkgTokenSpaceGuid.PcdVariableStoreSize|0x40000
!endif

!if $(NETWORK_TLS_ENABLE) == TRUE
  gEfiMdeModulePkgTokenSpaceGuid.PcdVariableStoreSize|0x80000
  gEfiMdeModulePkgTokenSpaceGuid.PcdMaxVolatileVariableSize|0x40000
!endif

  gEfiMdeModulePkgTokenSpaceGuid.PcdVpdBaseAddress|0x0
  gEfiMdePkgTokenSpaceGuid.PcdReportStatusCodePropertyMask|0x07
  gEfiMdeModulePkgTokenSpaceGuid.PcdStatusCodeUseSerial|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdStatusCodeUseMemory|TRUE
  gPcAtChipsetPkgTokenSpaceGuid.PcdUartIoPortBaseAddress|0x402

  # DEBUG_INIT      0x00000001  // Initialization
  # DEBUG_WARN      0x00000002  // Warnings
  # DEBUG_LOAD      0x00000004  // Load events
  # DEBUG_FS        0x00000008  // EFI File system
  # DEBUG_POOL      0x00000010  // Alloc & Free (pool)
  # DEBUG_PAGE      0x00000020  // Alloc & Free (page)
  # DEBUG_INFO      0x00000040  // Informational debug messages
  # DEBUG_DISPATCH  0x00000080  // PEI/DXE/SMM Dispatchers
  # DEBUG_VARIABLE  0x00000100  // Variable
  # DEBUG_BM        0x00000400  // Boot Manager
  # DEBUG_BLKIO     0x00001000  // BlkIo Driver
  # DEBUG_NET       0x00004000  // SNP Driver
  # DEBUG_UNDI      0x00010000  // UNDI Driver
  # DEBUG_LOADFILE  0x00020000  // LoadFile
  # DEBUG_EVENT     0x00080000  // Event messages
  # DEBUG_GCD       0x00100000  // Global Coherency Database changes
  # DEBUG_CACHE     0x00200000  // Memory range cachability changes
  # DEBUG_VERBOSE   0x00400000  // Detailed debug messages that may
  #                             // significantly impact boot performance
  # DEBUG_ERROR     0x80000000  // Error
   gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x80080246
  #gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x800002CF # use when debugging depex loading issues
  gEfiMdePkgTokenSpaceGuid.PcdFixedDebugPrintErrorLevel|gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel

  # This PCD is used to set the base address of the PCI express hierarchy. It
  # is only consulted when OVMF runs on Q35. In that case it is programmed into
  # the PCIEXBAR register.
  #
  # On Q35 machine types that QEMU intends to support in the long term, QEMU
  # never lets the RAM below 4 GB exceed 2816 MB.
  gEfiMdePkgTokenSpaceGuid.PcdPciExpressBaseAddress|0xB0000000
  gUefiCpuPkgTokenSpaceGuid.PcdCpuMaxLogicalProcessorNumber|$(QEMU_CORE_NUM)

!if $(SMM_ENABLED) == FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdEmuVariableNvModeEnable|TRUE
!endif

  # Use profile index 1
  gOemPkgTokenSpaceGuid.PcdActiveProfileIndex|0x1

[PcdsFixedAtBuild.common]
  # a PCD that controls the enumeration and connection of ConIn's. When true, ConIn is only connected once a console input is requests
  gEfiMdeModulePkgTokenSpaceGuid.PcdConInConnectOnDemand|TRUE

# Enable SHELL to build instead of just taking the binary
  gEfiMdePkgTokenSpaceGuid.PcdUefiLibMaxPrintBufferSize|16000
  gEfiShellPkgTokenSpaceGuid.PcdShellProfileMask|0x1f    # All profiles

  gMsGraphicsPkgTokenSpaceGuid.PcdUiThemeInDxe|TRUE
  gEfiMdeModulePkgTokenSpaceGuid.PcdBootManagerInBootOrder|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdPlatformRecoverySupport|FALSE
  gPcBdsPkgTokenSpaceGuid.PcdLowResolutionInternalShell|FALSE
  # Set this to be gOemConfigPolicyGuid
  gSetupDataPkgTokenSpaceGuid.PcdConfigurationPolicyGuid|{GUID("ba320ade-e132-4c99-a3df-74d673ea6f76")}
  # The GUID of SetupDataPkg/ConfApp/ConfApp.inf: E3624086-4FCD-446E-9D07-B6B913792071

!if $(GUI_FRONT_PAGE) == TRUE
# Note:
#  The PcdBootManagerMenuFile Pcd does two things:
#    1.  It sets the default front page (the one entered exiting the shell, or if all boot
#        boot options fail), and:
#    2.  If this application is the first boot option being loaded, Bds does not call ReadyToBoot
#        to lock the private settings variables.
#
#  To use this feature:
#
#    1. Build QemuQ35Pkg with BLD_*_GUI_FRONT_PAGE=TRUE
#    2. Run QEMU

#    3. Exit from the shell to enter the gui front page. Because the shell was booted first,
#       the private settings variables will still be locked, and the boot order is unlocked.
#    4. Reorder the the UEFI front page boot option to be the first boot option using GUI (click
#       the mouse in the Q35 display to give Qemu the mouse pointer, move the mouse pointer to
#       the MU UEFI UI Front Page entry,  press the left button down, while keeping the left
#       button down, move the mouse pointer to the first boot option (probably Internal Stoarage),
#       then let go of the left button. The GUI is very slow, so wait for it).
#    5. Select Exit, Restart to restart the system to enter front page with the variables unlocked

#
#  Until you build again, every time you start Q35, the GUI front page will load first.
#
  gEfiMdeModulePkgTokenSpaceGuid.PcdBootManagerMenuFile|{ 0x8A, 0x70, 0x42, 0x40, 0x2D, 0x0F, 0x23, 0x48, 0xAC, 0x60, 0x0D, 0x77, 0xB3, 0x11, 0x18, 0x89 }
!else
  gEfiMdeModulePkgTokenSpaceGuid.PcdBootManagerMenuFile|{ 0x86, 0x40, 0x62, 0xe3, 0xcd, 0x4f, 0x6e, 0x44, 0x9d, 0x7, 0xb6, 0xb9, 0x13, 0x79, 0x20, 0x71 }
!endif
  gQemuPkgTokenSpaceGuid.PcdUIApplicationFile|{ 0x8A, 0x70, 0x42, 0x40, 0x2D, 0x0F, 0x23, 0x48, 0xAC, 0x60, 0x0D, 0x77, 0xB3, 0x11, 0x18, 0x89 }

  gUefiQemuQ35PkgTokenSpaceGuid.PcdOvmfFlashVariablesEnable|TRUE
  gQemuPkgTokenSpaceGuid.PcdOvmfHostBridgePciDevId|0x29C0

  # CMOS region is 128 bytes
  gMsWheaPkgTokenSpaceGuid.PcdMsWheaReportEarlyStorageCapacity|0x80

[PcdsFixedAtBuild.IA32]
  #
  # The NumberOfPages values below are ad-hoc. They are updated sporadically at
  # best (please refer to git-blame for past updates). The values capture a set
  # of BIN hints that made sense at a particular time, for some (now likely
  # unknown) workloads / boot paths.
  #
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiACPIMemoryNVS|0x80
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiACPIReclaimMemory|0x20
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiReservedMemoryType|0x510
!if $(SMM_ENABLED) == FALSE
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiRuntimeServicesCode|0x200
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiRuntimeServicesData|0x200
!else
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiRuntimeServicesCode|0x100
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiRuntimeServicesData|0x1C0
!endif

[PcdsFixedAtBuild.X64]

  #
  # Network Pcds
  #
!include NetworkPkg/NetworkPcds.dsc.inc
  gUefiCpuPkgTokenSpaceGuid.PcdCpuSmmStackSize|0x4000
  gUefiCpuPkgTokenSpaceGuid.PcdCpuSmmSyncMode|0x00
  gUefiCpuPkgTokenSpaceGuid.PcdCpuSmmApSyncTimeout|1000000


  # IRQs 5, 9, 10, 11 are level-triggered
  #gUefiTempTokenSpaceGuid.Pcd8259LegacyModeEdgeLevel|0x0E20
  gUefiQemuQ35PkgTokenSpaceGuid.Pcd8259LegacyModeEdgeLevel|0x0E20

  gMsGraphicsPkgTokenSpaceGuid.PcdMsGopOverrideProtocolGuid|{0xF5, 0x3B, 0x5E, 0xAA, 0x8A, 0x81, 0x2D, 0x41, 0xA1, 0x8E, 0xD8, 0x79, 0x3B, 0xA0, 0x3A, 0x5C}

  # QEMU does not support SMRR, adding the PCD override to skip corresponding audit tests
  !if $(BUILD_UNIT_TESTS) == TRUE
    gUefiTestingPkgTokenSpaceGuid.PcdPlatformSmrrUnsupported|TRUE
  !endif

################################################################################
#
# Pcd Dynamic Section - list of all EDK II PCD Entries defined by this Platform
#
################################################################################

[PcdsDynamicDefault]

  gEfiMdeModulePkgTokenSpaceGuid.PcdPciDisableBusEnumeration|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdVideoHorizontalResolution|1024
  gEfiMdeModulePkgTokenSpaceGuid.PcdVideoVerticalResolution|768
  gEfiMdeModulePkgTokenSpaceGuid.PcdAcpiS3Enable|FALSE
  gUefiQemuQ35PkgTokenSpaceGuid.PcdPciMmio64Size|0x800000000
  gUefiQemuQ35PkgTokenSpaceGuid.PcdPciIoBase|0x0
  gUefiQemuQ35PkgTokenSpaceGuid.PcdPciIoSize|0x0
  gUefiQemuQ35PkgTokenSpaceGuid.PcdPciMmio32Base|0x0
  gUefiQemuQ35PkgTokenSpaceGuid.PcdPciMmio32Size|0x0
  gUefiQemuQ35PkgTokenSpaceGuid.PcdPciMmio64Base|0x0
  gUefiQemuQ35PkgTokenSpaceGuid.PcdPciMmio64Size|0x800000000

  # Limit to SHA256 for now.
  gEfiSecurityPkgTokenSpaceGuid.PcdTpm2HashMask|0x00002

  gEfiMdeModulePkgTokenSpaceGuid.PcdEmuVariableNvStoreReserved|0

  gEfiMdePkgTokenSpaceGuid.PcdPlatformBootTimeOut|0

  # Set video resolution for text setup.
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoHorizontalResolution|1024
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoVerticalResolution|768
  # Set video resolution source to be controlled by video driver
  gQemuPkgTokenSpaceGuid.PcdVideoResolutionSource|2

  gEfiMdeModulePkgTokenSpaceGuid.PcdSmbiosVersion|0x0208
  gEfiMdeModulePkgTokenSpaceGuid.PcdSmbiosDocRev|0x0
  gUefiQemuQ35PkgTokenSpaceGuid.PcdQemuSmbiosValidated|FALSE

  # UefiCpuPkg PCDs related to initial AP bringup and general AP management.
  gUefiCpuPkgTokenSpaceGuid.PcdCpuBootLogicalProcessorNumber|0

  gUefiQemuQ35PkgTokenSpaceGuid.PcdQ35TsegMbytes|8
  gUefiQemuQ35PkgTokenSpaceGuid.PcdQ35SmramAtDefaultSmbase|FALSE


  gEfiSecurityPkgTokenSpaceGuid.PcdOptionRomImageVerificationPolicy|0x00

!if $(TPM_ENABLE) == TRUE
  # Set this to gEfiTpmDeviceInstanceTpm20DtpmGuid
  gEfiSecurityPkgTokenSpaceGuid.PcdTpmInstanceGuid|{0x5a, 0xf2, 0x6b, 0x28, 0xc3, 0xc2, 0x8c, 0x40, 0xb3, 0xb4, 0x25, 0xe6, 0x75, 0x8b, 0x73, 0x17}
!endif

  # IPv4 and IPv6 PXE Boot support.
  gEfiNetworkPkgTokenSpaceGuid.PcdIPv4PXESupport|0x01
  gEfiNetworkPkgTokenSpaceGuid.PcdIPv6PXESupport|0x01

  # Set ConfidentialComputing defaults
  gEfiMdePkgTokenSpaceGuid.PcdConfidentialComputingGuestAttr|0

  # Add DEVICE_STATE_UNIT_TEST_MODE to the device state bitmask if BUILD_UNIT_TESTS=TRUE (default)
  !if $(BUILD_UNIT_TESTS) == TRUE
    gEfiMdeModulePkgTokenSpaceGuid.PcdDeviceStateBitmask|0x20
  !endif

  # Set to debug if debugger is enabled.
  !if $(DEBUGGER_ENABLED) == TRUE
    gEfiMdeModulePkgTokenSpaceGuid.PcdDeviceStateBitmask|0x08
  !endif

[PcdsDynamicHii]
!if $(TPM_ENABLE) == TRUE && $(TPM_CONFIG_ENABLE) == TRUE
  gEfiSecurityPkgTokenSpaceGuid.PcdTcgPhysicalPresenceInterfaceVer|L"TCG2_VERSION"|gTcg2ConfigFormSetGuid|0x0|"1.3"|NV,BS
  gEfiSecurityPkgTokenSpaceGuid.PcdTpm2AcpiTableRev|L"TCG2_VERSION"|gTcg2ConfigFormSetGuid|0x8|3|NV,BS
!endif

################################################################################
#
# MFCI DSC include - packaged this way because MFCI aspires to one day be a binary
#
################################################################################
!include MfciPkg/MfciPkg.dsc.inc

################################################################################
#
# Components Section - list of all EDK II Modules needed by this Platform.
#
################################################################################
[Components]
  !include $(SHARED_CRYPTO_PATH)/Driver/Bin/CryptoDriver.inc.dsc

QemuQ35Pkg/Library/PeiFvMeasurementExclusionLib/PeiFvMeasurementExclusionLib.inf

QemuQ35Pkg/Library/ResetSystemLib/BaseResetSystemLib.inf
QemuQ35Pkg/Library/ResetSystemLib/DxeResetSystemLib.inf
QemuQ35Pkg/Library/ResetSystemLib/StandaloneMmResetSystemLib.inf

[Components.IA32]
  QemuQ35Pkg/ResetVector/ResetVector.inf

  #########################################
  # SEC Phase modules
  #########################################
  QemuQ35Pkg/Sec/SecMain.inf {
    <LibraryClasses>
      NULL|MdeModulePkg/Library/LzmaCustomDecompressLib/LzmaCustomDecompressLib.inf
      NULL|MdePkg/Library/StackCheckLibNull/StackCheckLibNull.inf
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
  UefiCpuPkg/Universal/Acpi/S3Resume2Pei/S3Resume2Pei.inf {
    <LibraryClasses>

      LockBoxLib|MdeModulePkg/Library/SmmLockBoxLib/SmmLockBoxPeiLib.inf

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

[Components.X64]
  #########################################
  # DXE Phase modules
  #########################################
  # Reads smbios type 3 to determine volume button state.
  QemuPkg/FrontPageButtons/FrontPageButtons.inf

  # Application that presents and manages FrontPage.
  OemPkg/FrontPage/FrontPage.inf

  # Application that presents & manages the Boot Menu Setup on Front Page.
  OemPkg/BootMenu/BootMenu.inf

  PcBdsPkg/MsBootPolicy/MsBootPolicy.inf

  # Apply Variable Policy to Load Option UEFI Variables
  MsCorePkg/LoadOptionVariablePolicyDxe/LoadOptionVariablePolicyDxe.inf

  MdeModulePkg/Universal/BootManagerPolicyDxe/BootManagerPolicyDxe.inf

  # AuthManager provides authentication for DFCI. AuthManagerNull passes out a consistent token to allow the rest
  # of FrontPage to be developed and tested while RngLib or other parts of the authentication process are being developed.
  DfciPkg/IdentityAndAuthManager/IdentityAndAuthManagerDxe.inf

  # Processes ingoing and outgoing DFCI settings requests.
  DfciPkg/DfciManager/DfciManager.inf

  # Manages windows and fonts to be drawn by the RenderingEngine.
  MsGraphicsPkg/SimpleWindowManagerDxe/SimpleWindowManagerDxe.inf
  # Produces EfiGraphicsOutputProtocol to draw graphics to the screen.
  MsGraphicsPkg/RenderingEngineDxe/RenderingEngineDxe.inf

  # Driver for On Screen Keyboard.
  MsGraphicsPkg/OnScreenKeyboardDxe/OnScreenKeyboardDxe.inf

  # Installs protocol to share the UI theme. If PcdUiThemeInDxe, this will involve calling the PlatformThemeLib directly.
  # Otherwise, the theme will have been generated in PEI and it will be located on a HOB.
  MsGraphicsPkg/MsUiTheme/Dxe/MsUiThemeProtocol.inf

  # Produces FORM DISPLAY ENGINE protocol. Handles input, displays strings.
  MsGraphicsPkg/DisplayEngineDxe/DisplayEngineDxe.inf


  MdeModulePkg/Core/Dxe/DxeMain.inf {
    <LibraryClasses>
      NULL|MdeModulePkg/Library/LzmaCustomDecompressLib/LzmaCustomDecompressLib.inf
      DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  }

  MdeModulePkg/Universal/ReportStatusCodeRouter/RuntimeDxe/ReportStatusCodeRouterRuntimeDxe.inf
  MdeModulePkg/Universal/StatusCodeHandler/RuntimeDxe/StatusCodeHandlerRuntimeDxe.inf
  MdeModulePkg/Universal/PCD/Dxe/Pcd.inf  {
   <LibraryClasses>
      PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  }

  MdeModulePkg/Core/RuntimeDxe/RuntimeDxe.inf

  # CPU branding information
  QemuQ35Pkg/CpuInfoDxe/CpuInfoDxe.inf

  MdeModulePkg/Universal/SecurityStubDxe/SecurityStubDxe.inf {
    <LibraryClasses>
!if $(TPM_REPLAY_ENABLED) == TRUE
      NULL|TpmTestingPkg/Overrides/Library/DxeImageVerificationLib/DxeImageVerificationLib.inf
!else
      NULL|SecurityPkg/Library/DxeImageVerificationLib/DxeImageVerificationLib.inf
!endif

!if $(TPM_ENABLE) == TRUE
!if $(TPM_REPLAY_ENABLED) == TRUE
      # Only TPM 2.0 is supported by the feature
      NULL|TpmTestingPkg/Overrides/Library/DxeTpm2MeasureBootLib/DxeTpm2MeasureBootLib.inf
!else
      NULL|SecurityPkg/Library/DxeTpmMeasureBootLib/DxeTpmMeasureBootLib.inf
      NULL|SecurityPkg/Library/DxeTpm2MeasureBootLib/DxeTpm2MeasureBootLib.inf
!endif
!endif
  }

  MmSupervisorPkg/Drivers/MmSupervisorRing3Broker/MmSupervisorRing3Broker.inf
  MmSupervisorPkg/Drivers/StandaloneMmUnblockMem/StandaloneMmUnblockMem.inf

  QemuQ35Pkg/8259InterruptControllerDxe/8259.inf
  UefiCpuPkg/CpuIo2Dxe/CpuIo2Dxe.inf
  UefiCpuPkg/CpuDxe/CpuDxe.inf {
    <LibraryClasses>
    NULL|MsCorePkg/Library/MemoryProtectionExceptionHandlerLib/MemoryProtectionExceptionHandlerLib.inf
  }
  QemuQ35Pkg/8254TimerDxe/8254Timer.inf
  QemuQ35Pkg/IncompatiblePciDeviceSupportDxe/IncompatiblePciDeviceSupport.inf
  QemuPkg/PciHotPlugInitDxe/PciHotPlugInit.inf
  MdeModulePkg/Bus/Pci/PciHostBridgeDxe/PciHostBridgeDxe.inf {
    <LibraryClasses>
      PciHostBridgeLib|QemuQ35Pkg/Library/PciHostBridgeLib/PciHostBridgeLib.inf
      PciHostBridgeUtilityLib|QemuQ35Pkg/Library/PciHostBridgeUtilityLib/PciHostBridgeUtilityLib.inf
      NULL|QemuQ35Pkg/Library/PlatformHasIoMmuLib/PlatformHasIoMmuLib.inf
  }
  MdeModulePkg/Bus/Pci/PciBusDxe/PciBusDxe.inf {
    <LibraryClasses>
      PcdLib|MdePkg/Library/DxePcdLib/DxePcdLib.inf
  }
  MdeModulePkg/Universal/ResetSystemRuntimeDxe/ResetSystemRuntimeDxe.inf
  MdeModulePkg/Universal/Metronome/Metronome.inf
  PcAtChipsetPkg/PcatRealTimeClockRuntimeDxe/PcatRealTimeClockRuntimeDxe.inf
  MdeModulePkg/Universal/DriverHealthManagerDxe/DriverHealthManagerDxe.inf
  MdeModulePkg/Universal/BdsDxe/BdsDxe.inf {
    <PcdsDynamicExDefault>
      gMsGraphicsPkgTokenSpaceGuid.PcdPostBackgroundColoringSkipCount|0
  }

  QemuQ35Pkg/QemuKernelLoaderFsDxe/QemuKernelLoaderFsDxe.inf {
    <LibraryClasses>
      NULL|QemuQ35Pkg/Library/BlobVerifierLibNull/BlobVerifierLibNull.inf
  }
  QemuPkg/VirtioPciDeviceDxe/VirtioPciDeviceDxe.inf
  QemuPkg/Virtio10Dxe/Virtio10.inf
  QemuPkg/VirtioBlkDxe/VirtioBlk.inf
  QemuPkg/VirtioScsiDxe/VirtioScsi.inf
  QemuPkg/VirtioRngDxe/VirtioRng.inf

  # Rng Protocol producer
  SecurityPkg/RandomNumberGenerator/RngDxe/RngDxe.inf {
    <LibraryClasses>
      RngLib|MdeModulePkg/Library/BaseRngLibTimerLib/BaseRngLibTimerLib.inf
  }

  MdeModulePkg/Universal/WatchdogTimerDxe/WatchdogTimer.inf
  MdeModulePkg/Universal/MonotonicCounterRuntimeDxe/MonotonicCounterRuntimeDxe.inf
  MdeModulePkg/Universal/CapsuleRuntimeDxe/CapsuleRuntimeDxe.inf
  MdeModulePkg/Universal/Console/ConPlatformDxe/ConPlatformDxe.inf
  MdeModulePkg/Universal/Console/ConSplitterDxe/ConSplitterDxe.inf
  MdeModulePkg/Universal/Console/GraphicsConsoleDxe/GraphicsConsoleDxe.inf {
    <LibraryClasses>
      PcdLib|MdePkg/Library/DxePcdLib/DxePcdLib.inf
  }
  MdeModulePkg/Universal/Console/TerminalDxe/TerminalDxe.inf
  MdeModulePkg/Universal/DevicePathDxe/DevicePathDxe.inf {
    <LibraryClasses>
      DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
      PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  }
  MdeModulePkg/Universal/PrintDxe/PrintDxe.inf
  MdeModulePkg/Universal/Disk/DiskIoDxe/DiskIoDxe.inf
  MdeModulePkg/Universal/Disk/PartitionDxe/PartitionDxe.inf
  MdeModulePkg/Universal/Disk/RamDiskDxe/RamDiskDxe.inf
  MdeModulePkg/Universal/Disk/UnicodeCollation/EnglishDxe/EnglishDxe.inf
  FatPkg/EnhancedFatDxe/Fat.inf
  MdeModulePkg/Universal/Disk/UdfDxe/UdfDxe.inf
  MdeModulePkg/Bus/Scsi/ScsiBusDxe/ScsiBusDxe.inf
  MdeModulePkg/Bus/Scsi/ScsiDiskDxe/ScsiDiskDxe.inf
  QemuPkg/SataControllerDxe/SataControllerDxe.inf
  MdeModulePkg/Bus/Ata/AtaAtapiPassThru/AtaAtapiPassThru.inf
  MdeModulePkg/Bus/Ata/AtaBusDxe/AtaBusDxe.inf
  MdeModulePkg/Bus/Pci/NvmExpressDxe/NvmExpressDxe.inf
  MdeModulePkg/Universal/HiiDatabaseDxe/HiiDatabaseDxe.inf
  MdeModulePkg/Universal/SetupBrowserDxe/SetupBrowserDxe.inf
  MdeModulePkg/Universal/MemoryTest/NullMemoryTestDxe/NullMemoryTestDxe.inf
  AdvLoggerPkg/Application/AdvancedLogDumper/AdvancedLogDumper.inf

  QemuQ35Pkg/QemuVideoDxe/QemuVideoDxe.inf


  QemuQ35Pkg/QemuRamfbDxe/QemuRamfbDxe.inf

  #
  # ISA Support
  #
  QemuQ35Pkg/SioBusDxe/SioBusDxe.inf
  MdeModulePkg/Bus/Pci/PciSioSerialDxe/PciSioSerialDxe.inf
  MdeModulePkg/Bus/Isa/Ps2KeyboardDxe/Ps2KeyboardDxe.inf

  #
  # SMBIOS Support
  #
  MdeModulePkg/Universal/SmbiosDxe/SmbiosDxe.inf {
    <LibraryClasses>
      NULL|QemuQ35Pkg/Library/SmbiosVersionLib/DetectSmbiosVersionLib.inf
  }
  QemuQ35Pkg/SmbiosPlatformDxe/SmbiosPlatformDxe.inf

  #
  # ACPI Support
  #
  MdeModulePkg/Universal/Acpi/AcpiTableDxe/AcpiTableDxe.inf
  QemuQ35Pkg/AcpiPlatformDxe/AcpiPlatformDxe.inf
  MdeModulePkg/Universal/Acpi/S3SaveStateDxe/S3SaveStateDxe.inf
  MdeModulePkg/Universal/Acpi/BootScriptExecutorDxe/BootScriptExecutorDxe.inf
  MdeModulePkg/Universal/Acpi/BootGraphicsResourceTableDxe/BootGraphicsResourceTableDxe.inf

  #
  # Network Support
  #

  NetworkPkg/DpcDxe/DpcDxe.inf

  NetworkPkg/SnpDxe/SnpDxe.inf
  NetworkPkg/MnpDxe/MnpDxe.inf

  NetworkPkg/ArpDxe/ArpDxe.inf
  NetworkPkg/Dhcp4Dxe/Dhcp4Dxe.inf
  NetworkPkg/Ip4Dxe/Ip4Dxe.inf
  NetworkPkg/Udp4Dxe/Udp4Dxe.inf
  NetworkPkg/Mtftp4Dxe/Mtftp4Dxe.inf


  NetworkPkg/Dhcp6Dxe/Dhcp6Dxe.inf
  NetworkPkg/Ip6Dxe/Ip6Dxe.inf
  NetworkPkg/Udp6Dxe/Udp6Dxe.inf
  NetworkPkg/Mtftp6Dxe/Mtftp6Dxe.inf


  NetworkPkg/TcpDxe/TcpDxe.inf
  NetworkPkg/UefiPxeBcDxe/UefiPxeBcDxe.inf

  NetworkPkg/TlsDxe/TlsDxe.inf
  NetworkPkg/TlsAuthConfigDxe/TlsAuthConfigDxe.inf

  NetworkPkg/DnsDxe/DnsDxe.inf
  NetworkPkg/HttpDxe/HttpDxe.inf
  NetworkPkg/HttpUtilitiesDxe/HttpUtilitiesDxe.inf
  NetworkPkg/HttpBootDxe/HttpBootDxe.inf

  QemuPkg/VirtioNetDxe/VirtioNet.inf
  NetworkPkg/UefiPxeBcDxe/UefiPxeBcDxe.inf {
    <LibraryClasses>
      NULL|QemuQ35Pkg/Library/PxeBcPcdProducerLib/PxeBcPcdProducerLib.inf
  }

  #
  # HID Support
  #
  HidPkg/UefiHidDxe/UefiHidDxe.inf

  #
  # Usb Support
  #
  MdeModulePkg/Bus/Pci/UhciDxe/UhciDxe.inf
  MdeModulePkg/Bus/Pci/EhciDxe/EhciDxe.inf
  MdeModulePkg/Bus/Pci/XhciDxe/XhciDxe.inf
  MdeModulePkg/Bus/Usb/UsbBusDxe/UsbBusDxe.inf
  MdeModulePkg/Bus/Usb/UsbKbDxe/UsbKbDxe.inf
  MdeModulePkg/Bus/Usb/UsbMassStorageDxe/UsbMassStorageDxe.inf
  HidPkg/UsbHidDxe/UsbHidDxe.inf {
    <LibraryClasses>
      UefiUsbLib|MdePkg/Library/UefiUsbLib/UefiUsbLib.inf
  }

  ShellPkg/DynamicCommand/TftpDynamicCommand/TftpDynamicCommand.inf {
    <PcdsFixedAtBuild>
      gEfiShellPkgTokenSpaceGuid.PcdShellLibAutoInitialize|FALSE
  }
  ShellPkg/DynamicCommand/HttpDynamicCommand/HttpDynamicCommand.inf {
    <PcdsFixedAtBuild>
      gEfiShellPkgTokenSpaceGuid.PcdShellLibAutoInitialize|FALSE
  }
  ShellPkg/DynamicCommand/VariablePolicyDynamicCommand/VariablePolicyDynamicCommand.inf {
    <PcdsFixedAtBuild>
      gEfiShellPkgTokenSpaceGuid.PcdShellLibAutoInitialize|FALSE
  }
  QemuPkg/LinuxInitrdDynamicShellCommand/LinuxInitrdDynamicShellCommand.inf {
    <PcdsFixedAtBuild>
      gEfiShellPkgTokenSpaceGuid.PcdShellLibAutoInitialize|FALSE
  }
  ShellPkg/Application/Shell/Shell.inf {
    <LibraryClasses>
      ShellCommandLib|ShellPkg/Library/UefiShellCommandLib/UefiShellCommandLib.inf
      NULL|ShellPkg/Library/UefiShellLevel2CommandsLib/UefiShellLevel2CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellLevel1CommandsLib/UefiShellLevel1CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellLevel3CommandsLib/UefiShellLevel3CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellDriver1CommandsLib/UefiShellDriver1CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellDebug1CommandsLib/UefiShellDebug1CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellInstall1CommandsLib/UefiShellInstall1CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellNetwork1CommandsLib/UefiShellNetwork1CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellNetwork2CommandsLib/UefiShellNetwork2CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellAcpiViewCommandLib/UefiShellAcpiViewCommandLib.inf

      HandleParsingLib|ShellPkg/Library/UefiHandleParsingLib/UefiHandleParsingLib.inf
      PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
      BcfgCommandLib|ShellPkg/Library/UefiShellBcfgCommandLib/UefiShellBcfgCommandLib.inf

    <PcdsFixedAtBuild>
      gEfiShellPkgTokenSpaceGuid.PcdShellLibAutoInitialize|FALSE
      gEfiMdePkgTokenSpaceGuid.PcdUefiLibMaxPrintBufferSize|8000
    <PcdsPatchableInModule>
      #Turn off Halt on Assert and Print Assert so that libraries can
      #be tested in more of a release mode environment
      gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0xFF
  }

  PolicyServicePkg/PolicyService/DxeMm/PolicyDxe.inf

  SetupDataPkg/ConfApp/ConfApp.inf {
    <LibraryClasses>
      JsonLiteParserLib|MsCorePkg/Library/JsonLiteParser/JsonLiteParser.inf
  }

  QemuQ35Pkg/IoMmuDxe/IoMmuDxe.inf

  QemuQ35Pkg/CpuS3DataDxe/CpuS3DataDxe.inf

  AdvLoggerPkg/AdvancedFileLogger/AdvancedFileLogger.inf
  MsCorePkg/Universal/StatusCodeHandler/Serial/Dxe/SerialStatusCodeHandlerDxe.inf
  MsCorePkg/MuCryptoDxe/MuCryptoDxe.inf
  MdeModulePkg/Universal/Acpi/FirmwarePerformanceDataTableDxe/FirmwarePerformanceDxe.inf
  MsGraphicsPkg/MsEarlyGraphics/Dxe/MsEarlyGraphics.inf
  MsWheaPkg/MsWheaReport/Dxe/MsWheaReportDxe.inf
  MsWheaPkg/MsWheaReport/Smm/MsWheaReportStandaloneMm.inf
  MdeModulePkg/Universal/ReportStatusCodeRouter/Smm/ReportStatusCodeRouterStandaloneMm.inf
  MsCorePkg/Universal/StatusCodeHandler/Serial/Smm/SerialStatusCodeHandlerSmm.inf
  MdeModulePkg/Universal/Acpi/FirmwarePerformanceDataTableSmm/FirmwarePerformanceStandaloneMm.inf
  MsCorePkg/MuVarPolicyFoundationDxe/MuVarPolicyFoundationDxe.inf
  # COMMENTED OUT FOR OVMF
  #SecurityPkg/Tcg/Tcg2Smm/Tcg2Smm.inf {
  #  <LibraryClasses>
      ## MS NOTE: This is just the collection of the main parts of PhysicalPresenceCallback() into individual functions for simpler management.
  #    Tcg2PhysicalPresenceLib|SecurityPkg/Library/SmmTcg2PhysicalPresenceLib/SmmTcg2PhysicalPresenceLib.inf
  # }
  # SecurityPkg/Tcg/MemoryOverwriteControl/TcgMor.inf
  DfciPkg/SettingsManager/SettingsManagerDxe.inf {
    #Platform should add all it settings libs here
    <LibraryClasses>
      NULL|ZeroTouchPkg/Library/ZeroTouchSettings/ZeroTouchSettings.inf
      NULL|DfciPkg/Library/DfciSettingsLib/DfciSettingsLib.inf
      #NULL|DfciPkg/Library/DfciPasswordProvider/DfciPasswordProvider.inf
      NULL|DfciPkg/Library/DfciVirtualizationSettings/DfciVirtualizationSettings.inf
      NULL|DfciPkg/Library/DfciWpbtSettingLib/DfciWpbtSetting.inf
      NULL|DfciPkg/Library/DfciAssetTagSettingLib/DfciAssetTagSetting.inf
      DfciSettingPermissionLib|DfciPkg/Library/DfciSettingPermissionLib/DfciSettingPermissionLib.inf
      NULL|OemPkg/Library/MsBootManagerSettingsDxeLib/MsBootManagerSettingsDxeLib.inf
      NULL|OemPkg/Library/MsSecureBootModeSettingLib/MsSecureBootModeSettingLib.inf
    <PcdsFeatureFlag>
      gDfciPkgTokenSpaceGuid.PcdSettingsManagerInstallProvider|TRUE
  }
  MdeModulePkg/Universal/EsrtFmpDxe/EsrtFmpDxe.inf
  MsCorePkg/AcpiRGRT/AcpiRgrt.inf
  MsCorePkg/HelloWorldRustDxe/HelloWorldRustDxe.inf
  DfciPkg/Application/DfciMenu/DfciMenu.inf

  MsGraphicsPkg/PrintScreenLogger/PrintScreenLogger.inf
  SecurityPkg/Hash2DxeCrypto/Hash2DxeCrypto.inf

## Unit Tests
#
## Powershell script to discover unit tests:
#
## Get-ChildItem -Recurse -Force -File | Where-Object {($_.Name -like "*test*") -and ($_.Extension -eq ".inf")} | ^
## Where-Object {(Select-String -InputObject $_ -Pattern "MODULE_TYPE\s*=\s*UEFI_APPLICATION")} | ^
## ForEach-Object {$path = $_.FullName -replace '\\','/'; Write-Output $path}
!if $(BUILD_UNIT_TESTS) == TRUE

  AdvLoggerPkg/UnitTests/LineParser/LineParserTestApp.inf
  DfciPkg/UnitTests/DeviceIdTest/DeviceIdTestApp.inf
  # DfciPkg/UnitTests/DfciVarLockAudit/UEFI/DfciVarLockAuditTestApp.inf # DOESN'T PRODUCE OUTPUT
  FmpDevicePkg/Test/UnitTest/Library/FmpDependencyLib/FmpDependencyLibUnitTestApp.inf
  !if $(TARGET) == DEBUG
    # VARIABLE POLICY MUST BE UNLOCKED FOR THE TEST TO RUN (POLICY CAN ONLY REMAIN UNLOCKED ON DEBUG BUILDS)
    MdeModulePkg/Test/ShellTest/VariablePolicyFuncTestApp/VariablePolicyFuncTestApp.inf
  !endif
  UefiTestingPkg/FunctionalSystemTests/MemoryAttributeProtocolFuncTestApp/MemoryAttributeProtocolFuncTestApp.inf
  MdePkg/Test/UnitTest/Library/BaseLib/BaseLibUnitTestApp.inf
  MdePkg/Test/UnitTest/Library/BaseSafeIntLib/TestBaseSafeIntLibTestApp.inf
  MfciPkg/UnitTests/MfciPolicyParsingUnitTest/MfciPolicyParsingUnitTestApp.inf
  # MsCorePkg/UnitTests/JsonTest/JsonTestApp.inf # SOMETIMES RESULTS IN INFINITE LOOP
  MsCorePkg/UnitTests/MathLibUnitTest/MathLibUnitTestApp.inf
  # MsGraphicsPkg/UnitTests/SpinnerTest/SpinnerTest.inf # DOESN'T PRODUCE OUTPUT
  MsWheaPkg/Test/UnitTests/Library/LibraryClass/CheckHwErrRecHeaderTestApp.inf
  # MsWheaPkg/Test/UnitTests/MsWheaEarlyStorageUnitTestApp/MsWheaEarlyUnitTestApp.inf # CMOS REGION TOO SMALL TO STORE DATA
  MsWheaPkg/Test/UnitTests/MsWheaReportUnitTestApp/MsWheaReportUnitTestApp.inf
  MmSupervisorPkg/Test/MmPagingAuditTest/UEFI/MmPagingAuditApp.inf
  MmSupervisorPkg/Test/MmSupvRequestUnitTestApp/MmSupvRequestUnitTestApp.inf
  MdeModulePkg/Application/MpServicesTest/MpServicesTest.inf
  # MdeModulePkg/Application/SmiHandlerProfileInfo/SmiHandlerProfileAuditTestApp.inf # DOESN'T PRODUCE OUTPUT
  # ShellPkg/Application/ShellCTestApp/ShellCTestApp.inf # DOESN'T PRODUCE OUTPUT
  # ShellPkg/Application/ShellSortTestApp/ShellSortTestApp.inf # DOESN'T PRODUCE OUTPUT
  UnitTestFrameworkPkg/Library/UnitTestBootLibUsbClass/UnitTestBootLibUsbClass.inf
  UnitTestFrameworkPkg/Library/UnitTestPersistenceLibSimpleFileSystem/UnitTestPersistenceLibSimpleFileSystem.inf
  UefiTestingPkg/AuditTests/BootAuditTest/UEFI/BootAuditTestApp.inf
  # UefiTestingPkg/AuditTests/DMAProtectionAudit/UEFI/DMAIVRSProtectionUnitTestApp.inf # NOT APPLICABLE TO Q35
  UefiTestingPkg/AuditTests/PagingAudit/UEFI/DxePagingAuditTestApp.inf
  # UefiTestingPkg/AuditTests/PagingAudit/UEFI/SmmPagingAuditTestApp.inf # DOESN'T PRODUCE OUTPUT
  # UefiTestingPkg/AuditTests/TpmEventLogAudit/TpmEventLogAuditTestApp.inf # DOESN'T PRODUCE OUTPUT
  # UefiTestingPkg/AuditTests/UefiVarLockAudit/UEFI/UefiVarLockAuditTestApp.inf # DOESN'T PRODUCE OUTPUT
  UefiTestingPkg/FunctionalSystemTests/ExceptionPersistenceTestApp/ExceptionPersistenceTestApp.inf
  UefiTestingPkg/FunctionalSystemTests/MemmapAndMatTestApp/MemmapAndMatTestApp.inf
  # UefiTestingPkg/FunctionalSystemTests/SmmPagingProtectionsTest/App/SmmPagingProtectionsTestApp.inf # NOT YET SUPPORTED
  # MOR LOCK NOT COMPATIBLE WITH STANDALONE MM: https://bugzilla.tianocore.org/show_bug.cgi?id=3513
  # UefiTestingPkg/FunctionalSystemTests/MorLockTestApp/MorLockTestApp.inf
  UefiTestingPkg/FunctionalSystemTests/MemoryProtectionTest/App/DxeMemoryProtectionTestApp.inf
  # UefiTestingPkg/FunctionalSystemTests/MemoryProtectionTest/App/SmmMemoryProtectionTestApp.inf # NOT APPLICABLE TO Q35
  # UefiTestingPkg/FunctionalSystemTests/SmmPagingProtectionsTest/Smm/SmmPagingProtectionsTestSmm.inf # NOT APPLICABLE TO Q35
  # UefiTestingPkg/FunctionalSystemTests/SmmPagingProtectionsTest/Smm/SmmPagingProtectionsTestStandaloneMm.inf # NOT YET SUPPORTED
  # UefiTestingPkg/FunctionalSystemTests/MemoryProtectionTest/Driver/SmmMemoryProtectionTestDriver.inf # NOT APPLICABLE TO Q35
  # UefiTestingPkg/AuditTests/PagingAudit/UEFI/DxePagingAuditDriver.inf # TEST RUN VIA APPLICATION
  XmlSupportPkg/Test/UnitTest/XmlTreeLib/XmlTreeLibUnitTestApp.inf
  XmlSupportPkg/Test/UnitTest/XmlTreeQueryLib/XmlTreeQueryLibUnitTestApp.inf {
    <PcdsPatchableInModule>
      #Turn off Halt on Assert and Print Assert so that libraries can
      #be tested in more of a release mode environment
      gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x0E
  }
!endif
  #########################################
  # SMM Phase modules
  #########################################

  #
  # SMM Initial Program Load (a DXE_RUNTIME_DRIVER)
  #
!if $(PEI_MM_IPL_ENABLED) == TRUE
  MmSupervisorPkg/Drivers/MmPeiLaunchers/MmIplX64Relay.inf
  MmSupervisorPkg/Drivers/MmPeiLaunchers/MmDxeSupport.inf {
    <LibraryClasses>
      NULL|StandaloneMmPkg/Library/VariableMmDependency/VariableMmDependency.inf
  }
!else
  QemuQ35Pkg/SmmAccess/SmmAccess2Dxe.inf
  MmSupervisorPkg/Drivers/StandaloneMmIpl/PiSmmIpl.inf
!endif
  MmSupervisorPkg/Drivers/MmSupervisorErrorReport/MmSupervisorErrorReport.inf

  #
  # SMM_CORE
  #
  MmSupervisorPkg/Core/MmSupervisorCore.inf

  #
  # Privileged drivers (DXE_SMM_DRIVER modules)
  #
  UefiCpuPkg/CpuIo2Smm/CpuIo2StandaloneMm.inf
  MdeModulePkg/Universal/LockBox/SmmLockBox/SmmLockBox.inf {
    <LibraryClasses>
      LockBoxLib|MdeModulePkg/Library/SmmLockBoxLib/SmmLockBoxSmmLib.inf
  }

  QemuQ35Pkg/SmmControl2Dxe/SmmControl2Dxe.inf


  #
  # Variable driver stack (SMM)
  #
  QemuQ35Pkg/QemuFlashFvbServicesRuntimeDxe/FvbServicesStandaloneMm.inf
  MdeModulePkg/Universal/FaultTolerantWriteDxe/FaultTolerantWriteStandaloneMm.inf
  MdeModulePkg/Universal/Variable/RuntimeDxe/VariableStandaloneMm.inf {
    <LibraryClasses>
      NULL|MdeModulePkg/Library/VarCheckUefiLib/VarCheckUefiLib.inf
      # mu_change
      NULL|MdeModulePkg/Library/VarCheckPolicyLib/VarCheckPolicyLibStandaloneMm.inf
  }
  MdeModulePkg/Universal/Variable/RuntimeDxe/VariableSmmRuntimeDxe.inf

  #
  # Variable driver stack (NO SMM)
  #
  MdeModulePkg/Universal/FaultTolerantWriteDxe/FaultTolerantWriteDxe.inf
  MdeModulePkg/Universal/Variable/RuntimeDxe/VariableRuntimeDxe.inf

  #
  # TPM support
  #
!if $(TPM_ENABLE) == TRUE
!if $(TPM_REPLAY_ENABLED) == TRUE
  TpmTestingPkg/Overrides/Tcg2Dxe/Tcg2Dxe.inf {
    <LibraryClasses>
      Tpm2DeviceLib|SecurityPkg/Library/Tpm2DeviceLibRouter/Tpm2DeviceLibRouterDxe.inf
      NULL|SecurityPkg/Library/Tpm2DeviceLibDTpm/Tpm2InstanceLibDTpm.inf
      HashLib|SecurityPkg/Library/HashLibBaseCryptoRouter/HashLibBaseCryptoRouterDxe.inf
      NULL|SecurityPkg/Library/HashInstanceLibSha1/HashInstanceLibSha1.inf
      NULL|SecurityPkg/Library/HashInstanceLibSha256/HashInstanceLibSha256.inf
      NULL|SecurityPkg/Library/HashInstanceLibSha384/HashInstanceLibSha384.inf
      NULL|SecurityPkg/Library/HashInstanceLibSha512/HashInstanceLibSha512.inf
      NULL|SecurityPkg/Library/HashInstanceLibSm3/HashInstanceLibSm3.inf
  }
!else
  SecurityPkg/Tcg/Tcg2Dxe/Tcg2Dxe.inf {
    <LibraryClasses>
      Tpm2DeviceLib|SecurityPkg/Library/Tpm2DeviceLibRouter/Tpm2DeviceLibRouterDxe.inf
      NULL|SecurityPkg/Library/Tpm2DeviceLibDTpm/Tpm2InstanceLibDTpm.inf
      HashLib|SecurityPkg/Library/HashLibBaseCryptoRouter/HashLibBaseCryptoRouterDxe.inf
      NULL|SecurityPkg/Library/HashInstanceLibSha1/HashInstanceLibSha1.inf
      NULL|SecurityPkg/Library/HashInstanceLibSha256/HashInstanceLibSha256.inf
      NULL|SecurityPkg/Library/HashInstanceLibSha384/HashInstanceLibSha384.inf
      NULL|SecurityPkg/Library/HashInstanceLibSha512/HashInstanceLibSha512.inf
      NULL|SecurityPkg/Library/HashInstanceLibSm3/HashInstanceLibSm3.inf
  }
!endif
!endif
!if $(TPM_CONFIG_ENABLE) == TRUE AND $(TPM_ENABLE) == TRUE
  SecurityPkg/Tcg/Tcg2Config/Tcg2ConfigDxe.inf
!endif

  # PRM Configuration Driver
  PrmPkg/PrmConfigDxe/PrmConfigDxe.inf {
    <LibraryClasses>
      NULL|PrmPkg/Samples/PrmSampleAcpiParameterBufferModule/Library/DxeAcpiParameterBufferModuleConfigLib/DxeAcpiParameterBufferModuleConfigLib.inf
      NULL|PrmPkg/Samples/PrmSampleContextBufferModule/Library/DxeContextBufferModuleConfigLib/DxeContextBufferModuleConfigLib.inf
      NULL|PrmPkg/Samples/PrmSampleHardwareAccessModule/Library/DxeHardwareAccessModuleConfigLib/DxeHardwareAccessModuleConfigLib.inf
      NULL|AdvLoggerPkg/AdvLoggerOsConnectorPrm/Library/AdvLoggerOsConnectorPrmConfigLib/AdvLoggerOsConnectorPrmConfigLib.inf
  }

  #
  # Platform Runtime Mechanism (PRM) feature
  #

  # PRM Module Loader Driver
  PrmPkg/PrmLoaderDxe/PrmLoaderDxe.inf

  # PRM SSDT Installation Driver
  PrmPkg/PrmSsdtInstallDxe/PrmSsdtInstallDxe.inf

  # PRM Sample Modules
  PrmPkg/Samples/PrmSampleAcpiParameterBufferModule/PrmSampleAcpiParameterBufferModule.inf
  PrmPkg/Samples/PrmSampleHardwareAccessModule/PrmSampleHardwareAccessModule.inf
  PrmPkg/Samples/PrmSampleContextBufferModule/PrmSampleContextBufferModule.inf
  AdvLoggerPkg/AdvLoggerOsConnectorPrm/AdvLoggerOsConnectorPrm.inf

  # PRM Information UEFI Application
  PrmPkg/Application/PrmInfo/PrmInfo.inf

  # MfciDxe overrides
  MfciPkg/MfciDxe/MfciDxe.inf {
    <LibraryClasses>
      MfciRetrievePolicyLib|MfciPkg/Library/MfciRetrievePolicyLibViaHob/MfciRetrievePolicyLibViaHob.inf
      MfciDeviceIdSupportLib|MfciPkg/Library/MfciDeviceIdSupportLibSmbios/MfciDeviceIdSupportLibSmbios.inf
  }

!include TpmTestingPkg/TpmReplay.dsc.inc

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

[BuildOptions.IA32]
  MSFT:*_*_*_DLINK_FLAGS = /ALIGN:64

# Force PE/COFF sections to be aligned at 4KB boundaries to support page level
# protection of DXE_SMM_DRIVER/SMM_CORE modules
[BuildOptions.common.EDKII.DXE_SMM_DRIVER, BuildOptions.common.EDKII.DXE_RUNTIME_DRIVER, BuildOptions.common.EDKII.SMM_CORE, BuildOptions.common.EDKII.DXE_DRIVER, BuildOptions.common.EDKII.DXE_CORE, BuildOptions.common.EDKII.UEFI_DRIVER, BuildOptions.common.EDKII.UEFI_APPLICATION, BuildOptions.common.EDKII.MM_CORE_STANDALONE, BuildOptions.common.EDKII.MM_STANDALONE]
  MSFT:*_*_*_DLINK_FLAGS = /ALIGN:4096
  GCC:*_GCC5_*_DLINK_FLAGS = -z common-page-size=0x1000
  GCC:*_CLANGPDB_*_DLINK_FLAGS = /ALIGN:4096
