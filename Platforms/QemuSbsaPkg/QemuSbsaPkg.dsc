#
#  Copyright (c) 2011 - 2022, ARM Limited. All rights reserved.
#  Copyright (c) 2014, Linaro Limited. All rights reserved.
#  Copyright (c) 2015 - 2020, Intel Corporation. All rights reserved.
#  Copyright (c) Microsoft Corporation.
#
#  SPDX-License-Identifier: BSD-2-Clause-Patent
#
#

################################################################################
#
# Defines Section - statements that will be processed to create a Makefile.
#
################################################################################
[Defines]
  PLATFORM_NAME                  = QemuSbsaPkg
  PLATFORM_GUID                  = 37d7e986-f7e9-45c2-8067-e371421a626c
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/QemuSbsaPkg
  SUPPORTED_ARCHITECTURES        = AARCH64
  BUILD_TARGETS                  = DEBUG|RELEASE|NOOPT
  SKUID_IDENTIFIER               = DEFAULT
  FLASH_DEFINITION               = QemuSbsaPkg/QemuSbsaPkg.fdf

  #  DEBUG_INIT      0x00000001  // Initialization
  #  DEBUG_WARN      0x00000002  // Warnings
  #  DEBUG_LOAD      0x00000004  // Load events
  #  DEBUG_FS        0x00000008  // EFI File system
  #  DEBUG_POOL      0x00000010  // Alloc & Free (pool)
  #  DEBUG_PAGE      0x00000020  // Alloc & Free (page)
  #  DEBUG_INFO      0x00000040  // Informational debug messages
  #  DEBUG_DISPATCH  0x00000080  // PEI/DXE/SMM Dispatchers
  #  DEBUG_VARIABLE  0x00000100  // Variable
  #  DEBUG_BM        0x00000400  // Boot Manager
  #  DEBUG_BLKIO     0x00001000  // BlkIo Driver
  #  DEBUG_NET       0x00004000  // SNP Driver
  #  DEBUG_UNDI      0x00010000  // UNDI Driver
  #  DEBUG_LOADFILE  0x00020000  // LoadFile
  #  DEBUG_EVENT     0x00080000  // Event messages
  #  DEBUG_GCD       0x00100000  // Global Coherency Database changes
  #  DEBUG_CACHE     0x00200000  // Memory range cachability changes
  #  DEBUG_VERBOSE   0x00400000  // Detailed debug messages that may
  #                              // significantly impact boot performance
  #  DEBUG_ERROR     0x80000000  // Error
  DEFINE DEBUG_PRINT_ERROR_LEVEL = 0x80080246

!if $(TARGET) != NOOPT
  DEFINE FD_SIZE_IN_MB    = 2
!else
  DEFINE FD_SIZE_IN_MB    = 3
!endif

!if $(FD_SIZE_IN_MB) == 2
  DEFINE FD_SIZE          = 0x200000
  DEFINE FD_NUM_BLOCKS    = 0x200
!endif
!if $(FD_SIZE_IN_MB) == 3
  DEFINE FD_SIZE          = 0x300000
  DEFINE FD_NUM_BLOCKS    = 0x300
!endif

  #
  # Defines for default states.  These can be changed on the command line.
  # -D FLAG=VALUE
  #
  DEFINE TTY_TERMINAL            = FALSE
  DEFINE TPM2_ENABLE             = FALSE
  DEFINE TPM2_CONFIG_ENABLE      = FALSE
  DEFINE BUILD_UNIT_TESTS        = TRUE

  #
  # Network definition
  #
  DEFINE NETWORK_IP6_ENABLE              = FALSE
  DEFINE NETWORK_HTTP_BOOT_ENABLE        = FALSE
  DEFINE NETWORK_SNP_ENABLE              = FALSE
  DEFINE NETWORK_TLS_ENABLE              = FALSE
  DEFINE NETWORK_ALLOW_HTTP_CONNECTIONS  = TRUE
  DEFINE NETWORK_ISCSI_ENABLE            = TRUE

!if $(NETWORK_SNP_ENABLE) == TRUE
  !error "NETWORK_SNP_ENABLE is IA32/X64/EBC only"
!endif

!include NetworkPkg/NetworkDefines.dsc.inc

!include MdePkg/MdeLibs.dsc.inc

[LibraryClasses.common]
!if $(TARGET) == RELEASE
  DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
!else
  DebugLib|MdePkg/Library/BaseDebugLibSerialPort/BaseDebugLibSerialPort.inf
!endif
  DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf

  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  SafeIntLib|MdePkg/Library/BaseSafeIntLib/BaseSafeIntLib.inf
  BmpSupportLib|MdeModulePkg/Library/BaseBmpSupportLib/BaseBmpSupportLib.inf
  SynchronizationLib|MdePkg/Library/BaseSynchronizationLib/BaseSynchronizationLib.inf
  PerformanceLib|MdePkg/Library/BasePerformanceLibNull/BasePerformanceLibNull.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  PeCoffGetEntryPointLib|MdePkg/Library/BasePeCoffGetEntryPointLib/BasePeCoffGetEntryPointLib.inf
  PeCoffLib|MdePkg/Library/BasePeCoffLib/BasePeCoffLib.inf
  IoLib|MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsicArmVirt.inf
  UefiDecompressLib|MdePkg/Library/BaseUefiDecompressLib/BaseUefiDecompressLib.inf
  CpuLib|MdePkg/Library/BaseCpuLib/BaseCpuLib.inf

  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  HobLib|ArmVirtPkg/Library/ArmVirtDxeHobLib/ArmVirtDxeHobLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLibDevicePathProtocol/UefiDevicePathLibDevicePathProtocol.inf
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  DxeServicesTableLib|MdePkg/Library/DxeServicesTableLib/DxeServicesTableLib.inf
  DxeServicesLib|MdePkg/Library/DxeServicesLib/DxeServicesLib.inf
  UefiDriverEntryPoint|MdePkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.inf
  UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
  HiiLib|MdeModulePkg/Library/UefiHiiLib/UefiHiiLib.inf
  UefiHiiServicesLib|MdeModulePkg/Library/UefiHiiServicesLib/UefiHiiServicesLib.inf
  SortLib|MdeModulePkg/Library/UefiSortLib/UefiSortLib.inf
  ShellLib|ShellPkg/Library/UefiShellLib/UefiShellLib.inf
  ShellCEntryLib|ShellPkg/Library/UefiShellCEntryLib/UefiShellCEntryLib.inf
  FileHandleLib|MdePkg/Library/UefiFileHandleLib/UefiFileHandleLib.inf

  UefiRuntimeLib|MdePkg/Library/UefiRuntimeLib/UefiRuntimeLib.inf
  OrderedCollectionLib|MdePkg/Library/BaseOrderedCollectionRedBlackTreeLib/BaseOrderedCollectionRedBlackTreeLib.inf

  #
  # Ramdisk Requirements
  #
  FileExplorerLib|MdeModulePkg/Library/FileExplorerLib/FileExplorerLib.inf

  # Allow dynamic PCDs
  #
  PcdLib|MdePkg/Library/DxePcdLib/DxePcdLib.inf

  # use the accelerated BaseMemoryLibOptDxe by default, overrides for SEC/PEI below
  BaseMemoryLib|MdePkg/Library/BaseMemoryLibOptDxe/BaseMemoryLibOptDxe.inf

  # Networking Requirements
!include NetworkPkg/NetworkLibs.dsc.inc
!if $(NETWORK_TLS_ENABLE) == TRUE
  TlsLib|CryptoPkg/Library/TlsLib/TlsLib.inf
!endif

  NonDiscoverableDeviceRegistrationLib|MdeModulePkg/Library/NonDiscoverableDeviceRegistrationLib/NonDiscoverableDeviceRegistrationLib.inf

  #
  # It is not possible to prevent the ARM compiler from inserting calls to intrinsic functions.
  # This library provides the intrinsic functions such a compiler may generate calls to.
  #
  # NULL|ArmPkg/Library/CompilerIntrinsicsLib/CompilerIntrinsicsLib.inf   # MU_CHANGE
  NULL|MdePkg/Library/CompilerIntrinsicsLib/ArmCompilerIntrinsicsLib.inf  # MU_CHANGE

  # Add support for GCC stack protector
  NULL|MdePkg/Library/BaseStackCheckLib/BaseStackCheckLib.inf

  # ARM Architectural Libraries
  CacheMaintenanceLib|ArmPkg/Library/ArmCacheMaintenanceLib/ArmCacheMaintenanceLib.inf
  DefaultExceptionHandlerLib|ArmPkg/Library/DefaultExceptionHandlerLib/DefaultExceptionHandlerLib.inf
  CpuExceptionHandlerLib|ArmPkg/Library/ArmExceptionLib/ArmExceptionLib.inf
  ArmDisassemblerLib|ArmPkg/Library/ArmDisassemblerLib/ArmDisassemblerLib.inf
  ArmGicLib|ArmPkg/Drivers/ArmGic/ArmGicLib.inf
  ArmGicArchLib|ArmPkg/Library/ArmGicArchLib/ArmGicArchLib.inf
  ArmSmcLib|ArmPkg/Library/ArmSmcLib/ArmSmcLib.inf
  ArmHvcLib|ArmPkg/Library/ArmHvcLib/ArmHvcLib.inf
  ArmGenericTimerCounterLib|ArmPkg/Library/ArmGenericTimerVirtCounterLib/ArmGenericTimerVirtCounterLib.inf

  PlatformPeiLib|ArmPlatformPkg/PlatformPei/PlatformPeiLib.inf
  MemoryInitPeiLib|ArmPlatformPkg/MemoryInitPei/MemoryInitPeiLib.inf

  # ARM PL031 RTC Driver
  RealTimeClockLib|ArmPlatformPkg/Library/PL031RealTimeClockLib/PL031RealTimeClockLib.inf
  TimeBaseLib|EmbeddedPkg/Library/TimeBaseLib/TimeBaseLib.inf
  # ARM PL011 UART Driver
  PL011UartClockLib|ArmPlatformPkg/Library/PL011UartClockLib/PL011UartClockLib.inf
  PL011UartLib|ArmPlatformPkg/Library/PL011UartLib/PL011UartLib.inf
  SerialPortLib|ArmPlatformPkg/Library/PL011SerialPortLib/PL011SerialPortLib.inf

  #
  # Uncomment (and comment out the next line) For RealView Debugger. The Standard IO window
  # in the debugger will show load and unload commands for symbols. You can cut and paste this
  # into the command window to load symbols. We should be able to use a script to do this, but
  # the version of RVD I have does not support scripts accessing system memory.
  #
  #PeCoffExtraActionLib|ArmPkg/Library/RvdPeCoffExtraActionLib/RvdPeCoffExtraActionLib.inf
  PeCoffExtraActionLib|ArmPkg/Library/DebugPeCoffExtraActionLib/DebugPeCoffExtraActionLib.inf
  #PeCoffExtraActionLib|MdePkg/Library/BasePeCoffExtraActionLibNull/BasePeCoffExtraActionLibNull.inf

  DebugAgentLib|MdeModulePkg/Library/DebugAgentLibNull/DebugAgentLibNull.inf
  DebugAgentTimerLib|EmbeddedPkg/Library/DebugAgentTimerLibNull/DebugAgentTimerLibNull.inf

  # Flattened Device Tree (FDT) access library
  FdtLib|EmbeddedPkg/Library/FdtLib/FdtLib.inf

  # PCI Libraries
  PciLib|MdePkg/Library/BasePciLibPciExpress/BasePciLibPciExpress.inf
  PciExpressLib|MdePkg/Library/BasePciExpressLib/BasePciExpressLib.inf
  PciCapLib|QemuQ35Pkg/Library/BasePciCapLib/BasePciCapLib.inf
  PciCapPciSegmentLib|QemuQ35Pkg/Library/BasePciCapPciSegmentLib/BasePciCapPciSegmentLib.inf
  PciCapPciIoLib|QemuQ35Pkg/Library/UefiPciCapPciIoLib/UefiPciCapPciIoLib.inf

  # USB Libraries
  UefiUsbLib|MdePkg/Library/UefiUsbLib/UefiUsbLib.inf

  #
  # CryptoPkg libraries needed by multiple firmware features
  #
  IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
!if $(NETWORK_TLS_ENABLE) == TRUE
  OpensslLib|CryptoPkg/Library/OpensslLib/OpensslLib.inf
!else
  OpensslLib|CryptoPkg/Library/OpensslLib/OpensslLibCrypto.inf
!endif
  BaseCryptLib|CryptoPkg/Library/BaseCryptLib/BaseCryptLib.inf
  RngLib|MdePkg/Library/BaseRngLibTimerLib/BaseRngLibTimerLib.inf
  Hash2CryptoLib|SecurityPkg/Library/BaseHash2CryptoLibNull/BaseHash2CryptoLibNull.inf

  #
  # Secure Boot dependencies
  #
  AuthVariableLib|SecurityPkg/Library/AuthVariableLib/AuthVariableLib.inf
  SecureBootVariableLib|SecurityPkg/Library/SecureBootVariableLib/SecureBootVariableLib.inf
  SecureBootVariableProvisionLib|SecurityPkg/Library/SecureBootVariableProvisionLib/SecureBootVariableProvisionLib.inf
  # re-use the UserPhysicalPresent() dummy implementation from the ovmf tree
  PlatformSecureLib|QemuQ35Pkg/Library/PlatformSecureLib/PlatformSecureLib.inf

  # Security Libraries
  LockBoxLib            |QemuQ35Pkg/Library/LockBoxLib/LockBoxBaseLib.inf
  PasswordStoreLib      |MsCorePkg/Library/PasswordStoreLibNull/PasswordStoreLibNull.inf
  PasswordPolicyLib     |OemPkg/Library/PasswordPolicyLib/PasswordPolicyLib.inf
  SecureBootKeyStoreLib |OemPkg/Library/SecureBootKeyStoreLibOem/SecureBootKeyStoreLibOem.inf
  PlatformPKProtectionLib|SecurityPkg/Library/PlatformPKProtectionLibVarPolicy/PlatformPKProtectionLibVarPolicy.inf
  MuSecureBootKeySelectorLib|MsCorePkg/Library/MuSecureBootKeySelectorLib/MuSecureBootKeySelectorLib.inf

  # Variable Libraries
  VarCheckLib|MdeModulePkg/Library/VarCheckLib/VarCheckLib.inf
  VariablePolicyLib|MdeModulePkg/Library/VariablePolicyLib/VariablePolicyLib.inf
  VariablePolicyHelperLib|MdeModulePkg/Library/VariablePolicyHelperLib/VariablePolicyHelperLib.inf

  # Whea Libraries
  MsWheaEarlyStorageLib |MsWheaPkg/Library/MsWheaEarlyStorageLibNull/MsWheaEarlyStorageLibNull.inf
  MuTelemetryHelperLib  |MsWheaPkg/Library/MuTelemetryHelperLib/MuTelemetryHelperLib.inf

  UefiBootManagerLib|MdeModulePkg/Library/UefiBootManagerLib/UefiBootManagerLib.inf
  ReportStatusCodeLib|MdePkg/Library/BaseReportStatusCodeLibNull/BaseReportStatusCodeLibNull.inf

  # Unit Test Libs
  UnitTestLib             |UnitTestFrameworkPkg/Library/UnitTestLib/UnitTestLib.inf
  UnitTestBootLib         |UnitTestFrameworkPkg/Library/UnitTestBootLibNull/UnitTestBootLibNull.inf
  UnitTestPersistenceLib  |UnitTestFrameworkPkg/Library/UnitTestPersistenceLibSimpleFileSystem/UnitTestPersistenceLibSimpleFileSystem.inf
  UnitTestResultReportLib |XmlSupportPkg/Library/UnitTestResultReportJUnitFormatLib/UnitTestResultReportLib.inf

  # Base ARM libraries
  ArmLib|ArmPkg/Library/ArmLib/ArmBaseLib.inf
  ArmMmuLib|ArmPkg/Library/ArmMmuLib/ArmMmuBaseLib.inf # BEEBE TODO?
  MmuLib|ArmPkg/Library/MmuLib/BaseMmuLib.inf
  ArmSvcLib|ArmPkg/Library/ArmSvcLib/ArmSvcLib.inf

  # Virtio Support
  VirtioLib|QemuQ35Pkg/Library/VirtioLib/VirtioLib.inf

  ArmPlatformLib|ArmPlatformPkg/Library/ArmPlatformLibNull/ArmPlatformLibNull.inf

  TimerLib|ArmPkg/Library/ArmArchTimerLib/ArmArchTimerLib.inf

  CapsuleLib|MdeModulePkg/Library/DxeCapsuleLibNull/DxeCapsuleLibNull.inf
  BootLogoLib|MdeModulePkg/Library/BootLogoLib/BootLogoLib.inf
  PlatformBootManagerLib|MsCorePkg/Library/PlatformBootManagerLib/PlatformBootManagerLib.inf
  PlatformBmPrintScLib|QemuQ35Pkg/Library/PlatformBmPrintScLib/PlatformBmPrintScLib.inf
  CustomizedDisplayLib|MdeModulePkg/Library/CustomizedDisplayLib/CustomizedDisplayLib.inf
  FrameBufferBltLib|MdeModulePkg/Library/FrameBufferBltLib/FrameBufferBltLib.inf
  QemuBootOrderLib|QemuQ35Pkg/Library/QemuBootOrderLib/QemuBootOrderLib.inf
  FileExplorerLib|MdeModulePkg/Library/FileExplorerLib/FileExplorerLib.inf
  PciSegmentLib|MdePkg/Library/BasePciSegmentLibPci/BasePciSegmentLibPci.inf
  PciHostBridgeLib|QemuSbsaPkg/Library/SbsaQemuPciHostBridgeLib/SbsaQemuPciHostBridgeLib.inf

  # BEEBE ADDED
  MemoryTypeInformationChangeLib|MdeModulePkg/Library/MemoryTypeInformationChangeLibNull/MemoryTypeInformationChangeLibNull.inf
  BaseBinSecurityLib|MdePkg/Library/BaseBinSecurityLibNull/BaseBinSecurityLibNull.inf
  DxeMemoryProtectionHobLib|MdeModulePkg/Library/MemoryProtectionHobLibNull/DxeMemoryProtectionHobLibNull.inf
  MmMemoryProtectionHobLib|MdeModulePkg/Library/MemoryProtectionHobLibNull/MmMemoryProtectionHobLibNull.inf
  VariableFlashInfoLib|MdeModulePkg/Library/BaseVariableFlashInfoLib/BaseVariableFlashInfoLib.inf
  MemoryTypeInfoSecVarCheckLib|MdeModulePkg/Library/MemoryTypeInfoSecVarCheckLib/MemoryTypeInfoSecVarCheckLib.inf
  # ExceptionPersistenceLib|MsCorePkg/Library/ExceptionPersistenceLibCmos/ExceptionPersistenceLibCmos.inf
  SecurityLockAuditLib|MdeModulePkg/Library/SecurityLockAuditDebugMessageLib/SecurityLockAuditDebugMessageLib.inf
  ResetUtilityLib|MdeModulePkg/Library/ResetUtilityLib/ResetUtilityLib.inf
  HwResetSystemLib|ArmPkg/Library/ArmSmcPsciResetSystemLib/ArmSmcPsciResetSystemLib.inf
  FltUsedLib|MdePkg/Library/FltUsedLib/FltUsedLib.inf
  DeviceBootManagerLib|PcBdsPkg/Library/DeviceBootManagerLib/DeviceBootManagerLib.inf
  MsPlatformDevicesLib|QemuSbsaPkg/Library/MsPlatformDevicesLibQemuSbsa/MsPlatformDevicesLib.inf
  MsNetworkDependencyLib|PcBdsPkg/Library/MsNetworkDependencyLib/MsNetworkDependencyLib.inf
  MsBootOptionsLib|QemuQ35Pkg/Library/MsBootOptionsLibQemuQ35/MsBootOptionsLib.inf
  ConsoleMsgLib|PcBdsPkg/Library/ConsoleMsgLibNull/ConsoleMsgLibNull.inf
  MsBootPolicyLib|OemPkg/Library/MsBootPolicyLib/MsBootPolicyLib.inf
  MsBootManagerSettingsLib|PcBdsPkg/Library/MsBootManagerSettingsDxeLib/MsBootManagerSettingsDxeLib.inf
  MsPlatformPowerCheckLib|PcBdsPkg/Library/MsPlatformPowerCheckLibNull/MsPlatformPowerCheckLibNull.inf
  ThermalServicesLib|PcBdsPkg/Library/ThermalServicesLibNull/ThermalServicesLibNull.inf
  PowerServicesLib|PcBdsPkg/Library/PowerServicesLibNull/PowerServicesLibNull.inf
  MuUefiVersionLib|OemPkg/Library/MuUefiVersionLib/MuUefiVersionLib.inf
  MsNVBootReasonLib|OemPkg/Library/MsNVBootReasonLib/MsNVBootReasonLib.inf
  MuTelemetryHelperLib|MsWheaPkg/Library/MuTelemetryHelperLib/MuTelemetryHelperLib.inf
  UiRectangleLib|MsGraphicsPkg/Library/BaseUiRectangleLib/BaseUiRectangleLib.inf
  XenPlatformLib|QemuQ35Pkg/Library/XenPlatformLib/XenPlatformLib.inf
  MmUnblockMemoryLib|MdePkg/Library/MmUnblockMemoryLib/MmUnblockMemoryLibNull.inf
  ResetSystemLib|MdeModulePkg/Library/DxeResetSystemLib/DxeResetSystemLib.inf

  FdtHelperLib|QemuSbsaPkg/Library/FdtHelperLib/FdtHelperLib.inf
  OemMiscLib|QemuSbsaPkg/Library/OemMiscLib/OemMiscLib.inf

  # Math Libraries
  FltUsedLib |MdePkg/Library/FltUsedLib/FltUsedLib.inf
  MathLib    |MsCorePkg/Library/MathLib/MathLib.inf
  SafeIntLib |MdePkg/Library/BaseSafeIntLib/BaseSafeIntLib.inf

  # UI and graphics
  BmpSupportLib            |MdeModulePkg/Library/BaseBmpSupportLib/BaseBmpSupportLib.inf
  BootLogoLib              |MdeModulePkg/Library/BootLogoLib/BootLogoLib.inf
  BootGraphicsProviderLib  |OemPkg/Library/BootGraphicsProviderLib/BootGraphicsProviderLib.inf #  uses PCDs and raw files in the firmware volumes to get Pcd
  CustomizedDisplayLib     |MdeModulePkg/Library/CustomizedDisplayLib/CustomizedDisplayLib.inf
  FrameBufferBltLib        |MdeModulePkg/Library/FrameBufferBltLib/FrameBufferBltLib.inf
  FrameBufferMemDrawLib    |MsGraphicsPkg/Library/FrameBufferMemDrawLib/FrameBufferMemDrawLib.inf
  BootGraphicsLib          |MsGraphicsPkg/Library/BootGraphicsLib/BootGraphicsLib.inf
  GraphicsConsoleHelperLib |PcBdsPkg/Library/GraphicsConsoleHelperLib/GraphicsConsoleHelper.inf
  DisplayDeviceStateLib    |MsGraphicsPkg/Library/ColorBarDisplayDeviceStateLib/ColorBarDisplayDeviceStateLib.inf # Display the On screen notifications for the platform using color bars
  UIToolKitLib             |MsGraphicsPkg/Library/SimpleUIToolKit/SimpleUIToolKit.inf
  MsColorTableLib          |MsGraphicsPkg/Library/MsColorTableLib/MsColorTableLib.inf
  SwmDialogsLib            |MsGraphicsPkg/Library/SwmDialogsLib/SwmDialogs.inf #  Dialog Boxes in a Simple Window Manager environment.
  DeviceStateLib           |MdeModulePkg/Library/DeviceStateLib/DeviceStateLib.inf
  UiRectangleLib           |MsGraphicsPkg/Library/BaseUiRectangleLib/BaseUiRectangleLib.inf
  PlatformThemeLib         |QemuQ35Pkg/Library/PlatformThemeLib/PlatformThemeLib.inf # Q35 theme
  MsUiThemeCopyLib         |MsGraphicsPkg/Library/MsUiThemeCopyLib/MsUiThemeCopyLib.inf # handles copying the theme
  MsAltBootLib             |OemPkg/Library/MsAltBootLib/MsAltBootLib.inf
  MsPlatformEarlyGraphicsLib|MsGraphicsPkg/Library/MsEarlyGraphicsLibNull/Dxe/MsEarlyGraphicsLibNull.inf

  # Setup variable libraries
  ConfigBlobBaseLib         |SetupDataPkg/Library/ConfigBlobBaseLib/ConfigBlobBaseLib.inf
  ConfigDataLib             |SetupDataPkg/Library/ConfigDataLib/ConfigDataLib.inf
  ConfigVariableListLib     |SetupDataPkg/Library/ConfigVariableListLib/ConfigVariableListLib.inf
  ConfigSystemModeLib       |QemuQ35Pkg/Library/ConfigSystemModeLibQ35/ConfigSystemModeLib.inf
  ActiveProfileSelectorLib  |SetupDataPkg/Library/ActiveProfileSelectorLibNull/ActiveProfileSelectorLibNull.inf
  OemMfciLib|OemPkg/Library/OemMfciLib/OemMfciLibDxe.inf

  # DFCI / XML / JSON Libraries
  DfciUiSupportLib                  |DfciPkg/Library/DfciUiSupportLibNull/DfciUiSupportLibNull.inf # Supports DFCI Groups.
  DfciV1SupportLib                  |DfciPkg/Library/DfciV1SupportLibNull/DfciV1SupportLibNull.inf # Backwards compatibility with DFCI V1 functions.
  DfciDeviceIdSupportLib            |DfciPkg/Library/DfciDeviceIdSupportLibNull/DfciDeviceIdSupportLibNull.inf
  DfciGroupLib                      |DfciPkg/Library/DfciGroupLibNull/DfciGroups.inf
  DfciRecoveryLib                   |DfciPkg/Library/DfciRecoveryLib/DfciRecoveryLib.inf
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

  # TPM Libraries
  OemTpm2InitLib          |SecurityPkg/Library/OemTpm2InitLibNull/OemTpm2InitLib.inf
  Tpm2DebugLib            |SecurityPkg/Library/Tpm2DebugLib/Tpm2DebugLibNull.inf
  Tpm12CommandLib         |SecurityPkg/Library/Tpm12CommandLib/Tpm12CommandLib.inf
  Tpm2CommandLib          |SecurityPkg/Library/Tpm2CommandLib/Tpm2CommandLib.inf
  Tpm2DeviceLib           |SecurityPkg/Library/Tpm2DeviceLibTcg2/Tpm2DeviceLibTcg2.inf
  Tcg2PpVendorLib         |SecurityPkg/Library/Tcg2PpVendorLibNull/Tcg2PpVendorLibNull.inf
  TpmMeasurementLib       |MdeModulePkg/Library/TpmMeasurementLibNull/TpmMeasurementLibNull.inf
  Tcg2PhysicalPresenceLib |QemuQ35Pkg/Library/Tcg2PhysicalPresenceLibNull/DxeTcg2PhysicalPresenceLib.inf
!if $(TPM_ENABLE) == TRUE
  Tcg2PhysicalPresenceLib |QemuQ35Pkg/Library/Tcg2PhysicalPresenceLibQemu/DxeTcg2PhysicalPresenceLib.inf
  TpmMeasurementLib       |SecurityPkg/Library/DxeTpmMeasurementLib/DxeTpmMeasurementLib.inf
  Tpm2DebugLib            |SecurityPkg/Library/Tpm2DebugLib/Tpm2DebugLibSimple.inf
!endif

  # Common Standalone MM libraries
  MemLib|StandaloneMmPkg/Library/StandaloneMmMemLib/StandaloneMmMemLib.inf

[LibraryClasses.common.DXE_CORE, LibraryClasses.common.DXE_RUNTIME_DRIVER, LibraryClasses.common.UEFI_DRIVER, LibraryClasses.common.DXE_DRIVER, LibraryClasses.common.UEFI_APPLICATION]
  MsUiThemeLib                  |MsGraphicsPkg/Library/MsUiThemeLib/Dxe/MsUiThemeLib.inf

[LibraryClasses.common.SEC]
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf

  DebugAgentLib|ArmPkg/Library/DebugAgentSymbolsBaseLib/DebugAgentSymbolsBaseLib.inf
  HobLib|MdePkg/Library/PeiHobLib/PeiHobLib.inf
  PeiServicesLib|MdePkg/Library/PeiServicesLib/PeiServicesLib.inf
  PeiServicesTablePointerLib|ArmPkg/Library/PeiServicesTablePointerLib/PeiServicesTablePointerLib.inf
  MemoryAllocationLib|MdePkg/Library/PeiMemoryAllocationLib/PeiMemoryAllocationLib.inf

[LibraryClasses.common.PEI_CORE]
  PcdLib|MdePkg/Library/PeiPcdLib/PeiPcdLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  HobLib|MdePkg/Library/PeiHobLib/PeiHobLib.inf
  PeiServicesLib|MdePkg/Library/PeiServicesLib/PeiServicesLib.inf
  MemoryAllocationLib|MdePkg/Library/PeiMemoryAllocationLib/PeiMemoryAllocationLib.inf
  PeiCoreEntryPoint|MdePkg/Library/PeiCoreEntryPoint/PeiCoreEntryPoint.inf
  PerformanceLib|MdeModulePkg/Library/PeiPerformanceLib/PeiPerformanceLib.inf
  OemHookStatusCodeLib|MdeModulePkg/Library/OemHookStatusCodeLibNull/OemHookStatusCodeLibNull.inf
  PeCoffGetEntryPointLib|MdePkg/Library/BasePeCoffGetEntryPointLib/BasePeCoffGetEntryPointLib.inf
  ExtractGuidedSectionLib|MdePkg/Library/PeiExtractGuidedSectionLib/PeiExtractGuidedSectionLib.inf

  PeiServicesTablePointerLib|ArmPkg/Library/PeiServicesTablePointerLib/PeiServicesTablePointerLib.inf

[LibraryClasses.common.PEIM]
  PcdLib|MdePkg/Library/PeiPcdLib/PeiPcdLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  HobLib|MdePkg/Library/PeiHobLib/PeiHobLib.inf
  PeiServicesLib|MdePkg/Library/PeiServicesLib/PeiServicesLib.inf
  MemoryAllocationLib|MdePkg/Library/PeiMemoryAllocationLib/PeiMemoryAllocationLib.inf
  PeimEntryPoint|MdePkg/Library/PeimEntryPoint/PeimEntryPoint.inf
  PerformanceLib|MdeModulePkg/Library/PeiPerformanceLib/PeiPerformanceLib.inf
  OemHookStatusCodeLib|MdeModulePkg/Library/OemHookStatusCodeLibNull/OemHookStatusCodeLibNull.inf
  PeCoffGetEntryPointLib|MdePkg/Library/BasePeCoffGetEntryPointLib/BasePeCoffGetEntryPointLib.inf
  ExtractGuidedSectionLib|MdePkg/Library/PeiExtractGuidedSectionLib/PeiExtractGuidedSectionLib.inf
  ResetSystemLib|MdeModulePkg/Library/PeiResetSystemLib/PeiResetSystemLib.inf

  PeiServicesTablePointerLib|ArmPkg/Library/PeiServicesTablePointerLib/PeiServicesTablePointerLib.inf
  ArmVirtMemInfoLib|ArmVirtPkg/Library/QemuVirtMemInfoLib/QemuVirtMemInfoPeiLib.inf
  PcdDatabaseLoaderLib|MdeModulePkg/Library/PcdDatabaseLoaderLib/Pei/PcdDatabaseLoaderLibPei.inf

  # BEEBE ADDED
  MsPlatformEarlyGraphicsLib |MsGraphicsPkg/Library/MsEarlyGraphicsLibNull/Pei/MsEarlyGraphicsLibNull.inf
  MsUiThemeLib               |MsGraphicsPkg/Library/MsUiThemeLib/Pei/MsUiThemeLib.inf
  ArmPlatformLib             |QemuSbsaPkg/Library/SbsaQemuLib/SbsaQemuLib.inf
  OemMfciLib                 |OemPkg/Library/OemMfciLib/OemMfciLibPei.inf

!if $(TPM2_ENABLE) == TRUE
  BaseCryptLib|CryptoPkg/Library/BaseCryptLib/PeiCryptLib.inf
  Tpm2DeviceLib|SecurityPkg/Library/Tpm2DeviceLibDTpm/Tpm2DeviceLibDTpm.inf
!endif

[LibraryClasses.common.DXE_CORE]
  HobLib|MdePkg/Library/DxeCoreHobLib/DxeCoreHobLib.inf
  MemoryAllocationLib|MdeModulePkg/Library/DxeCoreMemoryAllocationLib/DxeCoreMemoryAllocationLib.inf
  DxeCoreEntryPoint|MdePkg/Library/DxeCoreEntryPoint/DxeCoreEntryPoint.inf
  ExtractGuidedSectionLib|MdePkg/Library/DxeExtractGuidedSectionLib/DxeExtractGuidedSectionLib.inf
  PerformanceLib|MdeModulePkg/Library/DxeCorePerformanceLib/DxeCorePerformanceLib.inf

[LibraryClasses.common.DXE_DRIVER]
  SecurityManagementLib|MdeModulePkg/Library/DxeSecurityManagementLib/DxeSecurityManagementLib.inf
  PerformanceLib|MdeModulePkg/Library/DxePerformanceLib/DxePerformanceLib.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  ReportStatusCodeLib|MdeModulePkg/Library/DxeReportStatusCodeLib/DxeReportStatusCodeLib.inf
  PcdDatabaseLoaderLib|MdeModulePkg/Library/PcdDatabaseLoaderLib/Dxe/PcdDatabaseLoaderLibDxe.inf
  UpdateFacsHardwareSignatureLib|PcBdsPkg/Library/UpdateFacsHardwareSignatureLib/UpdateFacsHardwareSignatureLib.inf
  LockBoxLib|MdeModulePkg/Library/SmmLockBoxLib/SmmLockBoxDxeLib.inf

!if $(TPM2_ENABLE) == TRUE
  Tpm2DeviceLib|SecurityPkg/Library/Tpm2DeviceLibTcg2/Tpm2DeviceLibTcg2.inf
!endif

[LibraryClasses.common.DXE_RUNTIME_DRIVER]
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  CapsuleLib|MdeModulePkg/Library/DxeCapsuleLibNull/DxeCapsuleLibNull.inf
!if $(TARGET) != RELEASE
  DebugLib|MdePkg/Library/DxeRuntimeDebugLibSerialPort/DxeRuntimeDebugLibSerialPort.inf
!endif
  VariablePolicyLib|MdeModulePkg/Library/VariablePolicyLib/VariablePolicyLibRuntimeDxe.inf
  ResetSystemLib|MdeModulePkg/Library/RuntimeResetSystemLib/RuntimeResetSystemLib.inf

!if $(SECURE_BOOT_ENABLE) == TRUE
  BaseCryptLib|CryptoPkg/Library/BaseCryptLib/RuntimeCryptLib.inf
!endif

[LibraryClasses.common.MM_CORE_STANDALONE]
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  ExtractGuidedSectionLib|EmbeddedPkg/Library/PrePiExtractGuidedSectionLib/PrePiExtractGuidedSectionLib.inf
  FvLib|StandaloneMmPkg/Library/FvLib/FvLib.inf
  HobLib|StandaloneMmPkg/Library/StandaloneMmCoreHobLib/StandaloneMmCoreHobLib.inf
  IoLib|MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf
  MemoryAllocationLib|StandaloneMmPkg/Library/StandaloneMmCoreMemoryAllocationLib/StandaloneMmCoreMemoryAllocationLib.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf

  ArmMmuLib|ArmPkg/Library/StandaloneMmMmuLib/ArmMmuStandaloneMmLib.inf
  StandaloneMmCoreEntryPoint|ArmPkg/Library/StandaloneMmCoreEntryPoint/StandaloneMmCoreEntryPoint.inf
  PeCoffExtraActionLib|StandaloneMmPkg/Library/StandaloneMmPeCoffExtraActionLib/StandaloneMmPeCoffExtraActionLib.inf

[LibraryClasses.common.MM_STANDALONE]
  StandaloneMmDriverEntryPoint|MdePkg/Library/StandaloneMmDriverEntryPoint/StandaloneMmDriverEntryPoint.inf

  HobLib|StandaloneMmPkg/Library/StandaloneMmHobLib/StandaloneMmHobLib.inf
  MmServicesTableLib|MdePkg/Library/StandaloneMmServicesTableLib/StandaloneMmServicesTableLib.inf
  MemoryAllocationLib|StandaloneMmPkg/Library/StandaloneMmMemoryAllocationLib/StandaloneMmMemoryAllocationLib.inf
  BaseCryptLib|CryptoPkg/Library/BaseCryptLib/SmmCryptLib.inf
  IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
  OpensslLib|CryptoPkg/Library/OpensslLib/OpensslLib.inf
  RngLib|MdePkg/Library/BaseRngLibTimerLib/BaseRngLibTimerLib.inf
  SynchronizationLib|MdePkg/Library/BaseSynchronizationLib/BaseSynchronizationLib.inf
  VarCheckLib|MdeModulePkg/Library/VarCheckLib/VarCheckLib.inf
  TimerLib|ArmPkg/Library/ArmArchTimerLib/ArmArchTimerLib.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf

  NorFlashPlatformLib|QemuSbsaPkg/Library/SbsaQemuNorFlashLib/SbsaQemuNorFlashLib.inf
  SafeIntLib|MdePkg/Library/BaseSafeIntLib/BaseSafeIntLib.inf
  AdvLoggerAccessLib|AdvLoggerPkg/Library/AdvLoggerMmAccessLib/AdvLoggerMmAccessLib.inf
  MemoryTypeInfoSecVarCheckLib|MdeModulePkg/Library/MemoryTypeInfoSecVarCheckLib/MemoryTypeInfoSecVarCheckLib.inf
  FltUsedLib|MdePkg/Library/FltUsedLib/FltUsedLib.inf

[LibraryClasses.common.UEFI_DRIVER]
  UefiScsiLib|MdePkg/Library/UefiScsiLib/UefiScsiLib.inf
  ExtractGuidedSectionLib|MdePkg/Library/DxeExtractGuidedSectionLib/DxeExtractGuidedSectionLib.inf
  PerformanceLib|MdeModulePkg/Library/DxePerformanceLib/DxePerformanceLib.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf

[LibraryClasses.common.UEFI_APPLICATION]
  PerformanceLib|MdeModulePkg/Library/DxePerformanceLib/DxePerformanceLib.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  HiiLib|MdeModulePkg/Library/UefiHiiLib/UefiHiiLib.inf

[BuildOptions]
!include NetworkPkg/NetworkBuildOptions.dsc.inc

################################################################################
#
# Pcd Section - list of all EDK II PCD Entries defined by this Platform
#
################################################################################

[PcdsFeatureFlag.common]
  gUefiQemuQ35PkgTokenSpaceGuid.PcdQemuBootOrderPciTranslation|TRUE
  gUefiQemuQ35PkgTokenSpaceGuid.PcdQemuBootOrderMmioTranslation|TRUE

  ## If TRUE, Graphics Output Protocol will be installed on virtual handle created by ConsplitterDxe.
  #  It could be set FALSE to save size.
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutGopSupport|TRUE
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutUgaSupport|FALSE

  gEfiMdeModulePkgTokenSpaceGuid.PcdTurnOffUsbLegacySupport|TRUE

  gEfiMdeModulePkgTokenSpaceGuid.PcdRequireIommu|FALSE # don't require IOMMU
  gEfiMdeModulePkgTokenSpaceGuid.PcdHiiOsRuntimeSupport|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdEnableVariableRuntimeCache|FALSE

  gArmVirtTokenSpaceGuid.PcdTpm2SupportEnabled|$(TPM2_ENABLE)

  gEmbeddedTokenSpaceGuid.PcdPrePiProduceMemoryTypeInformationHob|TRUE

[PcdsFeatureFlag.AARCH64]
  #
  # Activate AcpiSdtProtocol
  #
  gEfiMdeModulePkgTokenSpaceGuid.PcdInstallAcpiSdtProtocol|TRUE

[PcdsPatchableInModule]

  # DEBUG_ASSERT_ENABLED       0x01
  # DEBUG_PRINT_ENABLED        0x02
  # DEBUG_CODE_ENABLED         0x04
  # CLEAR_MEMORY_ENABLED       0x08
  # ASSERT_BREAKPOINT_ENABLED  0x10
  # ASSERT_DEADLOOP_ENABLED    0x20
!if $(TARGET) == RELEASE
  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x21
!else
  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0xff
!endif

[PcdsFixedAtBuild.common]

  gEfiMdePkgTokenSpaceGuid.PcdMaximumUnicodeStringLength|1000000
  gEfiMdePkgTokenSpaceGuid.PcdMaximumAsciiStringLength|1000000
  gEfiMdePkgTokenSpaceGuid.PcdMaximumLinkedListLength|0
  gEfiMdePkgTokenSpaceGuid.PcdSpinLockTimeout|10000000
  gEfiMdePkgTokenSpaceGuid.PcdUefiLibMaxPrintBufferSize|320

!if $(TARGET) != RELEASE
  gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|$(DEBUG_PRINT_ERROR_LEVEL)
!endif

  #
  # Optional feature to help prevent EFI memory map fragments
  # Turned on and off via: PcdPrePiProduceMemoryTypeInformationHob
  # Values are in EFI Pages (4K). DXE Core will make sure that
  # at least this much of each type of memory can be allocated
  # from a single memory range. This way you only end up with
  # maximum of two fragments for each type in the memory map
  # (the memory used, and the free memory that was prereserved
  # but not used).
  #
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiACPIReclaimMemory|0
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiACPIMemoryNVS|0
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiReservedMemoryType|0
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiRuntimeServicesData|600
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiRuntimeServicesCode|400
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiBootServicesCode|1500
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiBootServicesData|12000
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiLoaderCode|20
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiLoaderData|0

  #
  # Enable strict image permissions for all images. (This applies
  # only to images that were built with >= 4 KB section alignment.)
  #
#   gEfiMdeModulePkgTokenSpaceGuid.PcdImageProtectionPolicy|0x3

  #
  # Enable NX memory protection for all non-code regions, including OEM and OS
  # reserved ones, with the exception of LoaderData regions, of which OS loaders
  # (i.e., GRUB) may assume that its contents are executable.
  #
#   gEfiMdeModulePkgTokenSpaceGuid.PcdDxeNxMemoryProtectionPolicy|0xC000000000007FD1

  # BEEBE ADDED
  gMsGraphicsPkgTokenSpaceGuid.PcdUiThemeInDxe|TRUE
  gEfiMdeModulePkgTokenSpaceGuid.PcdBootManagerInBootOrder|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdPlatformRecoverySupport|FALSE
  gPcBdsPkgTokenSpaceGuid.PcdLowResolutionInternalShell|FALSE
  gMsGraphicsPkgTokenSpaceGuid.PcdMsGopOverrideProtocolGuid|{0xF5, 0x3B, 0x5E, 0xAA, 0x8A, 0x81, 0x2D, 0x41, 0xA1, 0x8E, 0xD8, 0x79, 0x3B, 0xA0, 0x3A, 0x5C}

!if $(ARCH) == AARCH64
  gArmTokenSpaceGuid.PcdVFPEnabled|1
!endif

  gArmPlatformTokenSpaceGuid.PcdCPUCoresStackBase|0x1000007c000
  gArmPlatformTokenSpaceGuid.PcdCPUCorePrimaryStackSize|0x4000
  gEfiMdeModulePkgTokenSpaceGuid.PcdMaxVariableSize|0x2000
  gEfiMdeModulePkgTokenSpaceGuid.PcdMaxAuthVariableSize|0x2800
  gEfiSecurityPkgTokenSpaceGuid.PcdUserPhysicalPresence|TRUE

!if $(NETWORK_TLS_ENABLE) == TRUE
  #
  # The cumulative and individual VOLATILE variable size limits should be set
  # high enough for accommodating several and/or large CA certificates.
  #
  gEfiMdeModulePkgTokenSpaceGuid.PcdVariableStoreSize|0x80000
  gEfiMdeModulePkgTokenSpaceGuid.PcdMaxVolatileVariableSize|0x40000
!endif

  # Size of the region used by UEFI in permanent memory (Reserved 64MB)
  gArmPlatformTokenSpaceGuid.PcdSystemMemoryUefiRegionSize|0x04000000
  gArmPlatformTokenSpaceGuid.PcdCoreCount|$(QEMU_CORE_NUM)

  #
  # ARM PrimeCell
  #

  ## PL011 - Serial Terminal
  gEfiMdeModulePkgTokenSpaceGuid.PcdSerialRegisterBase|0x60000000
  gEfiMdePkgTokenSpaceGuid.PcdUartDefaultBaudRate|115200

  ## Default Terminal Type
  ## 0-PCANSI, 1-VT100, 2-VT00+, 3-UTF8, 4-TTYTERM
!if $(TTY_TERMINAL) == TRUE
  gEfiMdePkgTokenSpaceGuid.PcdDefaultTerminalType|4
  # Set terminal type to TtyTerm, the value encoded is EFI_TTY_TERM_GUID
  gArmVirtTokenSpaceGuid.PcdTerminalTypeGuidBuffer|{0x80, 0x6d, 0x91, 0x7d, 0xb1, 0x5b, 0x8c, 0x45, 0xa4, 0x8f, 0xe2, 0x5f, 0xdd, 0x51, 0xef, 0x94}
!else
  gEfiMdePkgTokenSpaceGuid.PcdDefaultTerminalType|1
!endif

  #
  # ARM Virtual Architectural Timer -- fetch frequency from QEMU (TCG) or KVM
  #
  gArmTokenSpaceGuid.PcdArmArchTimerFreqInHz|0

  #
  # MM Communicate
  #
  gArmTokenSpaceGuid.PcdMmBufferBase|0x1003FE00000
  gArmTokenSpaceGuid.PcdMmBufferSize|0x200000

  #
  # PLDA PCI Root Complex
  #
  # ECAM size == 0x10000000
  gArmTokenSpaceGuid.PcdPciBusMin|0
  gArmTokenSpaceGuid.PcdPciBusMax|255
  gArmTokenSpaceGuid.PcdPciIoBase|0x0
  gArmTokenSpaceGuid.PcdPciIoSize|0x00010000
  gQemuSbsaPkgTokenSpaceGuid.PcdPciIoLimit|0x0000ffff
  gArmTokenSpaceGuid.PcdPciMmio32Base|0x80000000
  gArmTokenSpaceGuid.PcdPciMmio32Size|0x70000000
  gQemuSbsaPkgTokenSpaceGuid.PcdPciMmio32Limit|0xEFFFFFFF
  gArmTokenSpaceGuid.PcdPciMmio64Base|0x100000000
  gArmTokenSpaceGuid.PcdPciMmio64Size|0xFF00000000
  gQemuSbsaPkgTokenSpaceGuid.PcdPciMmio64Limit|0xFFFFFFFFFF

  # set PcdPciExpressBaseAddress to MAX_UINT64, which signifies that this
  # PCD and PcdPciDisableBusEnumeration have not been assigned yet
  # TODO: PcdPciExpressBaseAddress set to max_uint64
  gEfiMdePkgTokenSpaceGuid.PcdPciExpressBaseAddress|0xf0000000
  gQemuSbsaPkgTokenSpaceGuid.PcdPciExpressBarSize|0x10000000
  gQemuSbsaPkgTokenSpaceGuid.PcdPciExpressBarLimit|0xFFFFFFFF

  gEfiMdePkgTokenSpaceGuid.PcdPciIoTranslation|0x7fff0000
  gEfiMdePkgTokenSpaceGuid.PcdPciMmio32Translation|0x0
  gEfiMdePkgTokenSpaceGuid.PcdPciMmio64Translation|0x0
  ## If TRUE, OvmfPkg/AcpiPlatformDxe will not wait for PCI
  #  enumeration to complete before installing ACPI tables.
  gEfiMdeModulePkgTokenSpaceGuid.PcdPciDisableBusEnumeration|FALSE

  gDfciPkgTokenSpaceGuid.PcdUnsignedListFormatAllow|FALSE

  ## List of valid Profile GUIDs
  ## gQemuQ35PkgProfile1Guid, gQemuQ35PkgProfile2Guid, and gQemuQ35PkgProfile3Guid are supported.
  ## gSetupDataPkgGenericProfileGuid is defaulted to in case retrieved GUID is not in this list
  gSetupDataPkgTokenSpaceGuid.PcdConfigurationProfileList|{ GUID("E34D00D0-6A10-44BE-B46C-BEE6302C6287"), GUID("848F7E93-C957-4797-8A11-F301ED9B2048"), GUID("454CFA58-6423-4F50-8B2B-744BDECE876A") }

  #
  # Network Pcds
  #
!include NetworkPkg/NetworkPcds.dsc.inc

  # System Memory Base -- fixed at 0x100_0000_0000
  gArmTokenSpaceGuid.PcdSystemMemoryBase|0x10000000000

  # initial location of the device tree blob passed by QEMU -- base of DRAM
  gArmVirtTokenSpaceGuid.PcdDeviceTreeInitialBaseAddress|0x10000000000

  # Non discoverable devices (AHCI,EHCI)
  gQemuSbsaPkgTokenSpaceGuid.PcdPlatformAhciBase|0x60100000
  gQemuSbsaPkgTokenSpaceGuid.PcdPlatformAhciSize|0x00010000

  gEfiMdeModulePkgTokenSpaceGuid.PcdResetOnMemoryTypeInformationChange|FALSE
  # The GUID of SetupDataPkg/ConfApp/ConfApp.inf: E3624086-4FCD-446E-9D07-B6B913792071
  gEfiMdeModulePkgTokenSpaceGuid.PcdBootManagerMenuFile|{ 0x86, 0x40, 0x62, 0xe3, 0xcd, 0x4f, 0x6e, 0x44, 0x9d, 0x7, 0xb6, 0xb9, 0x13, 0x79, 0x20, 0x71 }
  # The GUID of Frontpage.inf from MU_OEM_SAMPLE: 4042708A-0F2D-4823-AC60-0D77B3111889
  gQemuQ35PkgTokenSpaceGuid.PcdUIApplicationFile|{ 0x8A, 0x70, 0x42, 0x40, 0x2D, 0x0F, 0x23, 0x48, 0xAC, 0x60, 0x0D, 0x77, 0xB3, 0x11, 0x18, 0x89 }

  #
  # The maximum physical I/O addressability of the processor, set with
  # BuildCpuHob().
  #
  gEmbeddedTokenSpaceGuid.PcdPrePiCpuIoSize|16

  #
  # Enable the non-executable DXE stack. (This gets set up by DxeIpl)
  #
#   gEfiMdeModulePkgTokenSpaceGuid.PcdSetNxForStack|TRUE

!if $(SECURE_BOOT_ENABLE) == TRUE
  # override the default values from SecurityPkg to ensure images from all sources are verified in secure boot
  gEfiSecurityPkgTokenSpaceGuid.PcdOptionRomImageVerificationPolicy|0x04
  gEfiSecurityPkgTokenSpaceGuid.PcdFixedMediaImageVerificationPolicy|0x04
  gEfiSecurityPkgTokenSpaceGuid.PcdRemovableMediaImageVerificationPolicy|0x04
!endif

  gEfiMdePkgTokenSpaceGuid.PcdReportStatusCodePropertyMask|0x0f
  gEfiShellPkgTokenSpaceGuid.PcdShellFileOperationSize|0x20000

  #
  # ARM General Interrupt Controller
  #
  gArmTokenSpaceGuid.PcdGicDistributorBase|0x40060000
  gArmTokenSpaceGuid.PcdGicRedistributorsBase|0x40080000

  # PPI #13
  gArmTokenSpaceGuid.PcdArmArchTimerSecIntrNum|29
  # PPI #14
  gArmTokenSpaceGuid.PcdArmArchTimerIntrNum|30
  # PPI #11
  gArmTokenSpaceGuid.PcdArmArchTimerVirtIntrNum|27
  # PPI #10
  gArmTokenSpaceGuid.PcdArmArchTimerHypIntrNum|26

[PcdsFixedAtBuild.common]
  gEfiMdeModulePkgTokenSpaceGuid.PcdAcpiDefaultOemId|"Palindrome"
  gEfiMdeModulePkgTokenSpaceGuid.PcdAcpiDefaultOemTableId|0x756D6551754D #MuQemuArm
  gEfiMdeModulePkgTokenSpaceGuid.PcdAcpiDefaultOemRevision|0x20221026
  gEfiMdeModulePkgTokenSpaceGuid.PcdAcpiDefaultCreatorId|0x554D5250 #PRMU
  gEfiMdeModulePkgTokenSpaceGuid.PcdAcpiDefaultCreatorRevision|1

[PcdsFixedAtBuild.AARCH64]
  # Clearing BIT0 in this PCD prevents installing a 32-bit SMBIOS entry point,
  # if the entry point version is >= 3.0. AARCH64 OSes cannot assume the
  # presence of the 32-bit entry point anyway (because many AARCH64 systems
  # don't have 32-bit addressable physical RAM), and the additional allocations
  # below 4 GB needlessly fragment the memory map. So expose the 64-bit entry
  # point only, for entry point versions >= 3.0.
  gEfiMdeModulePkgTokenSpaceGuid.PcdSmbiosEntryPointProvideMethod|0x2

[PcdsDynamicDefault.common]

  # System Memory Size -- 1 MB initially, actual size will be fetched from DT
  gArmTokenSpaceGuid.PcdSystemMemorySize|0x08000000

  ## PL031 RealTimeClock
  gArmPlatformTokenSpaceGuid.PcdPL031RtcBase|0x60010000

  #
  # Set video resolution for boot options and for text setup.
  # PlatformDxe can set the former at runtime.
  #
  gEfiMdeModulePkgTokenSpaceGuid.PcdVideoHorizontalResolution|1024
  gEfiMdeModulePkgTokenSpaceGuid.PcdVideoVerticalResolution|768
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoHorizontalResolution|1024
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoVerticalResolution|768
  # Set video resolution source to be controlled by video driver
  gUefiQemuQ35PkgTokenSpaceGuid.PcdVideoResolutionSource|2

  gEfiMdePkgTokenSpaceGuid.PcdPlatformBootTimeOut|0

  #
  # SMBIOS entry point version
  #
  gEfiMdeModulePkgTokenSpaceGuid.PcdSmbiosVersion|0x0304
  gEfiMdeModulePkgTokenSpaceGuid.PcdSmbiosDocRev|0x0

  gArmTokenSpaceGuid.PcdSystemBiosRelease|0x0100
  gArmTokenSpaceGuid.PcdEmbeddedControllerFirmwareRelease|0x0100

  gQemuSbsaPkgTokenSpaceGuid.PcdSystemManufacturer|L"Palindrome"
  gQemuSbsaPkgTokenSpaceGuid.PcdSystemSerialNumber|L"42-42-42-42"
  gQemuSbsaPkgTokenSpaceGuid.PcdSystemSKU|L"NorthAmerica"
  gQemuSbsaPkgTokenSpaceGuid.PcdSystemFamily|L"ArmMax"

  gQemuSbsaPkgTokenSpaceGuid.PcdBaseBoardAssetTag|L"ProjectMu"
  gQemuSbsaPkgTokenSpaceGuid.PcdBaseBoardSerialNumber|L"42-42-42-42"
  gQemuSbsaPkgTokenSpaceGuid.PcdBaseBoardSKU|L"NorthAmerica"
  gQemuSbsaPkgTokenSpaceGuid.PcdBaseBoardLocation|L"Internal"

  gQemuSbsaPkgTokenSpaceGuid.PcdChassisSerialNumber|L"42-42-42-42"
  gQemuSbsaPkgTokenSpaceGuid.PcdChassisVersion|L"1.0"
  gQemuSbsaPkgTokenSpaceGuid.PcdChassisManufacturer|L"Palindrome"
  gQemuSbsaPkgTokenSpaceGuid.PcdChassisAssetTag|L"ProjectMu"
  gQemuSbsaPkgTokenSpaceGuid.PcdChassisSKU|L"NorthAmerica"

  #
  # IPv4 and IPv6 PXE Boot support.
  #
  gEfiNetworkPkgTokenSpaceGuid.PcdIPv4PXESupport|0x01
  gEfiNetworkPkgTokenSpaceGuid.PcdIPv6PXESupport|0x01

  # Add DEVICE_STATE_UNIT_TEST_MODE to the device state bitmask if BUILD_UNIT_TESTS=TRUE (default)
  !if $(BUILD_UNIT_TESTS) == TRUE
    gEfiMdeModulePkgTokenSpaceGuid.PcdDeviceStateBitmask|0x20
  !endif

  #
  # TPM2 support
  #
  gEfiSecurityPkgTokenSpaceGuid.PcdTpmBaseAddress|0x0
!if $(TPM2_ENABLE) == TRUE
  gEfiSecurityPkgTokenSpaceGuid.PcdTpmInstanceGuid|{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
  gEfiSecurityPkgTokenSpaceGuid.PcdTpm2HashMask|0
!endif

[PcdsDynamicExDefault]
  # Default this to gSetupDataPkgGenericProfileGuid
  gSetupDataPkgTokenSpaceGuid.PcdSetupConfigActiveProfileFile|{ GUID("8464A6FF-A984-4899-A375-3DC1DB3D4227") }

  # Default this to gQemuQ35PkgDfciUnsignedXmlGuid
  gDfciPkgTokenSpaceGuid.PcdUnsignedPermissionsFile|{GUID("62CF29AD-FEEE-4930-B71B-4806C787C6AA")}

[PcdsDynamicHii]
  gArmVirtTokenSpaceGuid.PcdForceNoAcpi|L"ForceNoAcpi"|gArmVirtVariableGuid|0x0|FALSE|NV,BS

!if $(TPM2_CONFIG_ENABLE) == TRUE
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
# Components Section - list of all EDK II Modules needed by this Platform
#
################################################################################
[Components.common]
  #
  # PEI Phase modules
  #
  ArmPlatformPkg/PrePeiCore/PrePeiCoreUniCore.inf
  MdeModulePkg/Core/Pei/PeiMain.inf
  MdeModulePkg/Universal/PCD/Pei/Pcd.inf {
    <LibraryClasses>
      PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  }
  ArmPlatformPkg/PlatformPei/PlatformPeim.inf
  ArmPlatformPkg/MemoryInitPei/MemoryInitPeim.inf
  ArmPkg/Drivers/CpuPei/CpuPei.inf
  ArmPkg/Drivers/MmCommunicationPei/MmCommunicationPei.inf
  ArmPkg/Drivers/SmmVariablePei/SmmVariablePei.inf

  MdeModulePkg/Universal/Variable/Pei/VariablePei.inf

!if $(TPM2_ENABLE) == TRUE
  MdeModulePkg/Universal/ResetSystemPei/ResetSystemPei.inf
  QemuQ35Pkg/Tcg/Tcg2Config/Tcg2ConfigPei.inf
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

  MdeModulePkg/Core/DxeIplPeim/DxeIpl.inf
  MsCorePkg/Core/GuidedSectionExtractPeim/GuidedSectionExtract.inf {
    <LibraryClasses>
      NULL|MdeModulePkg/Library/LzmaCustomDecompressLib/LzmaCustomDecompressLib.inf
  }

  #
  # MU PEI Modules
  #
  MsWheaPkg/MsWheaReport/Pei/MsWheaReportPei.inf

  MsGraphicsPkg/MsUiTheme/Pei/MsUiThemePpi.inf
  MsGraphicsPkg/MsEarlyGraphics/Pei/MsEarlyGraphics.inf
  MdeModulePkg/Universal/Acpi/FirmwarePerformanceDataTablePei/FirmwarePerformancePei.inf
  OemPkg/DeviceStatePei/DeviceStatePei.inf
  MfciPkg/MfciPei/MfciPei.inf

  SetupDataPkg/ConfDfciUnsignedListInit/ConfDfciUnsignedListInit.inf
  PolicyServicePkg/PolicyService/Pei/PolicyPei.inf
  QemuQ35Pkg/ConfigDataGfx/ConfigDataGfx.inf

  #
  # DXE
  #
  MdeModulePkg/Core/Dxe/DxeMain.inf {
    <LibraryClasses>
      NULL|MdeModulePkg/Library/DxeCrc32GuidedSectionExtractLib/DxeCrc32GuidedSectionExtractLib.inf
      DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  }
  MdeModulePkg/Universal/PCD/Dxe/Pcd.inf {
    <LibraryClasses>
      PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  }

  #
  # Architectural Protocols
  #
  ArmPkg/Drivers/CpuDxe/CpuDxe.inf
  MdeModulePkg/Core/RuntimeDxe/RuntimeDxe.inf
  MdeModulePkg/Universal/Variable/RuntimeDxe/VariableSmmRuntimeDxe.inf
  MdeModulePkg/Universal/SecurityStubDxe/SecurityStubDxe.inf {
    <LibraryClasses>
      NULL|SecurityPkg/Library/DxeImageVerificationLib/DxeImageVerificationLib.inf
!if $(TPM2_ENABLE) == TRUE
      NULL|SecurityPkg/Library/DxeTpm2MeasureBootLib/DxeTpm2MeasureBootLib.inf
!endif
  }
  MdeModulePkg/Universal/CapsuleRuntimeDxe/CapsuleRuntimeDxe.inf
  MdeModulePkg/Universal/MonotonicCounterRuntimeDxe/MonotonicCounterRuntimeDxe.inf
  MdeModulePkg/Universal/ResetSystemRuntimeDxe/ResetSystemRuntimeDxe.inf
  EmbeddedPkg/RealTimeClockRuntimeDxe/RealTimeClockRuntimeDxe.inf
  EmbeddedPkg/MetronomeDxe/MetronomeDxe.inf

  MdeModulePkg/Universal/Console/ConPlatformDxe/ConPlatformDxe.inf
  MdeModulePkg/Universal/Console/ConSplitterDxe/ConSplitterDxe.inf
  MdeModulePkg/Universal/Console/GraphicsConsoleDxe/GraphicsConsoleDxe.inf
  MdeModulePkg/Universal/Console/TerminalDxe/TerminalDxe.inf
  MdeModulePkg/Universal/SerialDxe/SerialDxe.inf

  MdeModulePkg/Universal/HiiDatabaseDxe/HiiDatabaseDxe.inf

  ArmPkg/Drivers/ArmGic/ArmGicDxe.inf
  ArmPkg/Drivers/TimerDxe/TimerDxe.inf
  MdeModulePkg/Universal/WatchdogTimerDxe/WatchdogTimer.inf

  #
  # Status Code Routing
  #
  MdeModulePkg/Universal/ReportStatusCodeRouter/RuntimeDxe/ReportStatusCodeRouterRuntimeDxe.inf

  #
  # Platform Driver
  #
  EmbeddedPkg/Drivers/FdtClientDxe/FdtClientDxe.inf
  QemuQ35Pkg/VirtioBlkDxe/VirtioBlk.inf
  QemuQ35Pkg/VirtioScsiDxe/VirtioScsi.inf
  QemuQ35Pkg/VirtioNetDxe/VirtioNet.inf
  QemuQ35Pkg/VirtioRngDxe/VirtioRng.inf

  # Rng Protocol producer
  SecurityPkg/RandomNumberGenerator/RngDxe/RngDxe.inf

  #
  # FAT filesystem + GPT/MBR partitioning + UDF filesystem + virtio-fs
  #
  MdeModulePkg/Universal/Disk/DiskIoDxe/DiskIoDxe.inf
  MdeModulePkg/Universal/Disk/PartitionDxe/PartitionDxe.inf
  MdeModulePkg/Universal/Disk/UnicodeCollation/EnglishDxe/EnglishDxe.inf
  FatPkg/EnhancedFatDxe/Fat.inf
  MdeModulePkg/Universal/Disk/UdfDxe/UdfDxe.inf

  #
  # Bds
  #
  MdeModulePkg/Universal/DevicePathDxe/DevicePathDxe.inf {
    <LibraryClasses>
      DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
      PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  }

  # Spoofs button press to automatically boot to FrontPage.
  OemPkg/FrontpageButtonsVolumeUp/FrontpageButtonsVolumeUp.inf

  # Application that presents and manages FrontPage.
  OemPkg/FrontPage/FrontPage.inf

  # Application that presents & manages the Boot Menu Setup on Front Page.
  OemPkg/BootMenu/BootMenu.inf

  # BEEBE ADDED START
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
  QemuQ35Pkg/QemuVideoDxe/QemuVideoDxe.inf
  # BEEBE ADDED END

  MdeModulePkg/Universal/Acpi/FirmwarePerformanceDataTableDxe/FirmwarePerformanceDxe.inf
  MsCorePkg/MuCryptoDxe/MuCryptoDxe.inf
  MsGraphicsPkg/MsEarlyGraphics/Dxe/MsEarlyGraphics.inf
  MsWheaPkg/MsWheaReport/Dxe/MsWheaReportDxe.inf
  MsCorePkg/MuVarPolicyFoundationDxe/MuVarPolicyFoundationDxe.inf
  MsCorePkg/AcpiRGRT/AcpiRgrt.inf

  MsGraphicsPkg/PrintScreenLogger/PrintScreenLogger.inf
  SecurityPkg/Hash2DxeCrypto/Hash2DxeCrypto.inf

  #
  # DFCI support
  #
  # AuthManager provides authentication for DFCI. AuthManagerNull passes out a consistent token to allow the rest
  # of FrontPage to be developed and tested while RngLib or other parts of the authentication process are being developed.
  DfciPkg/IdentityAndAuthManager/IdentityAndAuthManagerDxe.inf

  # Processes ingoing and outgoing DFCI settings requests.
  DfciPkg/DfciManager/DfciManager.inf

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
      NULL|PcBdsPkg/Library/MsBootManagerSettingsDxeLib/MsBootManagerSettingsDxeLib.inf
      NULL|OemPkg/Library/MsSecureBootModeSettingLib/MsSecureBootModeSettingLib.inf
    <PcdsFeatureFlag>
      gDfciPkgTokenSpaceGuid.PcdSettingsManagerInstallProvider|TRUE
  }
  DfciPkg/Application/DfciMenu/DfciMenu.inf

  MdeModulePkg/Universal/SetupBrowserDxe/SetupBrowserDxe.inf
  MdeModulePkg/Universal/DriverHealthManagerDxe/DriverHealthManagerDxe.inf
  MdeModulePkg/Universal/BdsDxe/BdsDxe.inf {
    <PcdsDynamicExDefault>
      gMsGraphicsPkgTokenSpaceGuid.PcdPostBackgroundColoringSkipCount|0
  }
  PcBdsPkg/MsBootPolicy/MsBootPolicy.inf

  # Configuration modules
  PolicyServicePkg/PolicyService/Dxe/PolicyDxe.inf
  SetupDataPkg/ConfDataSettingProvider/ConfDataSettingProvider.inf {
    <PcdsFixedAtBuild>
      gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x80000000
  }
  SetupDataPkg/ConfProfileMgrDxe/ConfProfileMgrDxe.inf
  SetupDataPkg/ConfApp/ConfApp.inf {
    <LibraryClasses>
      JsonLiteParserLib|MsCorePkg/Library/JsonLiteParser/JsonLiteParser.inf
  }

  # MfciDxe overrides
  MfciPkg/MfciDxe/MfciDxe.inf {
    <LibraryClasses>
      MfciRetrievePolicyLib|MfciPkg/Library/MfciRetrievePolicyLibViaHob/MfciRetrievePolicyLibViaHob.inf
      MfciDeviceIdSupportLib|MfciPkg/Library/MfciDeviceIdSupportLibSmbios/MfciDeviceIdSupportLibSmbios.inf
  }

  #
  # Networking stack
  #
!include NetworkPkg/Network.dsc.inc

  #
  # SCSI Bus and Disk Driver
  #
  MdeModulePkg/Bus/Scsi/ScsiBusDxe/ScsiBusDxe.inf
  MdeModulePkg/Bus/Scsi/ScsiDiskDxe/ScsiDiskDxe.inf

  # IDE/AHCI Support
  QemuQ35Pkg/SataControllerDxe/SataControllerDxe.inf
  MdeModulePkg/Bus/Ata/AtaAtapiPassThru/AtaAtapiPassThru.inf
  MdeModulePkg/Bus/Ata/AtaBusDxe/AtaBusDxe.inf

  #
  # NVME Driver
  #
  MdeModulePkg/Bus/Pci/NvmExpressDxe/NvmExpressDxe.inf

  #
  # SMBIOS Support
  #
  MdeModulePkg/Universal/SmbiosDxe/SmbiosDxe.inf
  QemuSbsaPkg/SbsaQemuSmbiosDxe/SbsaQemuSmbiosDxe.inf
  ArmPkg/Universal/Smbios/ProcessorSubClassDxe/ProcessorSubClassDxe.inf
  ArmPkg/Universal/Smbios/SmbiosMiscDxe/SmbiosMiscDxe.inf

  #
  # PCI support
  #
  ArmPkg/Drivers/ArmPciCpuIo2Dxe/ArmPciCpuIo2Dxe.inf
  MdeModulePkg/Bus/Pci/PciHostBridgeDxe/PciHostBridgeDxe.inf
  MdeModulePkg/Bus/Pci/PciBusDxe/PciBusDxe.inf
  QemuQ35Pkg/PciHotPlugInitDxe/PciHotPlugInit.inf
  QemuQ35Pkg/VirtioPciDeviceDxe/VirtioPciDeviceDxe.inf
  QemuQ35Pkg/Virtio10Dxe/Virtio10.inf

  #
  # USB Support
  #
  MdeModulePkg/Bus/Pci/UhciDxe/UhciDxe.inf
  MdeModulePkg/Bus/Pci/EhciDxe/EhciDxe.inf
  MdeModulePkg/Bus/Pci/XhciDxe/XhciDxe.inf
  MdeModulePkg/Bus/Usb/UsbBusDxe/UsbBusDxe.inf
  MdeModulePkg/Bus/Usb/UsbKbDxe/UsbKbDxe.inf
  MdeModulePkg/Bus/Usb/UsbMassStorageDxe/UsbMassStorageDxe.inf
  MdeModulePkg/Bus/Usb/UsbMouseAbsolutePointerDxe/UsbMouseAbsolutePointerDxe.inf

  #
  # TPM2 support
  #
!if $(TPM2_ENABLE) == TRUE
  SecurityPkg/Tcg/Tcg2Dxe/Tcg2Dxe.inf {
    <LibraryClasses>
      HashLib|SecurityPkg/Library/HashLibBaseCryptoRouter/HashLibBaseCryptoRouterDxe.inf
      Tpm2DeviceLib|SecurityPkg/Library/Tpm2DeviceLibRouter/Tpm2DeviceLibRouterDxe.inf
      NULL|SecurityPkg/Library/Tpm2DeviceLibDTpm/Tpm2InstanceLibDTpm.inf
      NULL|SecurityPkg/Library/HashInstanceLibSha1/HashInstanceLibSha1.inf
      NULL|SecurityPkg/Library/HashInstanceLibSha256/HashInstanceLibSha256.inf
      NULL|SecurityPkg/Library/HashInstanceLibSha384/HashInstanceLibSha384.inf
      NULL|SecurityPkg/Library/HashInstanceLibSha512/HashInstanceLibSha512.inf
      NULL|SecurityPkg/Library/HashInstanceLibSm3/HashInstanceLibSm3.inf
  }
!if $(TPM2_CONFIG_ENABLE) == TRUE
  SecurityPkg/Tcg/Tcg2Config/Tcg2ConfigDxe.inf
!endif
!endif

  #
  # Ramdisk support
  #
  MdeModulePkg/Universal/Disk/RamDiskDxe/RamDiskDxe.inf

  ## Unit Tests
  !if $(BUILD_UNIT_TESTS) == TRUE
    # TODO: This is not supported on ARM yet
    # UefiTestingPkg/FunctionalSystemTests/MemoryProtectionTest/App/MemoryProtectionTestApp.inf
    # UefiTestingPkg/AuditTests/PagingAudit/UEFI/DxePagingAuditDriver.inf
    # UefiTestingPkg/AuditTests/PagingAudit/UEFI/DxePagingAuditTestApp.inf
    # UefiTestingPkg/FunctionalSystemTests/ExceptionPersistenceTestApp/ExceptionPersistenceTestApp.inf
    # UefiTestingPkg/FunctionalSystemTests/VarPolicyUnitTestApp/VarPolicyUnitTestApp.inf
    CryptoPkg/Test/UnitTest/Library/BaseCryptLib/BaseCryptLibUnitTestApp.inf {
      <PcdsPatchableInModule>
        #Turn off Halt on Assert and Print Assert so that libraries can
        #be tested in more of a release mode environment
        gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x0E
    }

    MsCorePkg/UnitTests/JsonTest/JsonTestApp.inf
    XmlSupportPkg/Test/UnitTest/XmlTreeLib/XmlTreeLibUnitTestApp.inf
    XmlSupportPkg/Test/UnitTest/XmlTreeQueryLib/XmlTreeQueryLibUnitTestApp.inf {
      <PcdsPatchableInModule>
        #Turn off Halt on Assert and Print Assert so that libraries can
        #be tested in more of a release mode environment
        gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x0E
    }

    # MemMap and MAT Test
    UefiTestingPkg/FunctionalSystemTests/MemmapAndMatTestApp/MemmapAndMatTestApp.inf
    # MorLock v1 and v2 Test
    # this fails on QEMU - UefiTestingPkg/FunctionalSystemTests/MorLockTestApp/MorLockTestApp.inf
  !endif

  #
  # Shell support
  #
  ShellPkg/DynamicCommand/TftpDynamicCommand/TftpDynamicCommand.inf {
    <PcdsFixedAtBuild>
      gEfiShellPkgTokenSpaceGuid.PcdShellLibAutoInitialize|FALSE
  }
  ShellPkg/DynamicCommand/HttpDynamicCommand/HttpDynamicCommand.inf {
    <PcdsFixedAtBuild>
      gEfiShellPkgTokenSpaceGuid.PcdShellLibAutoInitialize|FALSE
  }
  QemuQ35Pkg/LinuxInitrdDynamicShellCommand/LinuxInitrdDynamicShellCommand.inf {
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
!if $(ACPIVIEW_ENABLE) == TRUE
      NULL|ShellPkg/Library/UefiShellAcpiViewCommandLib/UefiShellAcpiViewCommandLib.inf
!endif
      NULL|ShellPkg/Library/UefiShellInstall1CommandsLib/UefiShellInstall1CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellNetwork1CommandsLib/UefiShellNetwork1CommandsLib.inf
!if $(NETWORK_IP6_ENABLE) == TRUE
      NULL|ShellPkg/Library/UefiShellNetwork2CommandsLib/UefiShellNetwork2CommandsLib.inf
!endif
      HandleParsingLib|ShellPkg/Library/UefiHandleParsingLib/UefiHandleParsingLib.inf
      PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
      BcfgCommandLib|ShellPkg/Library/UefiShellBcfgCommandLib/UefiShellBcfgCommandLib.inf
      ShellLib|ShellPkg/Library/UefiShellLib/UefiShellLib.inf

    <PcdsPatchableInModule>
      gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0xFF

    <PcdsFixedAtBuild>
      gEfiShellPkgTokenSpaceGuid.PcdShellLibAutoInitialize|FALSE
      gEfiMdePkgTokenSpaceGuid.PcdUefiLibMaxPrintBufferSize|8000
  }
  QemuSbsaPkg/SbsaQemuPlatformDxe/SbsaQemuPlatformDxe.inf
  MdeModulePkg/Bus/Pci/NonDiscoverablePciDeviceDxe/NonDiscoverablePciDeviceDxe.inf

[Components.AARCH64]
  #
  # ACPI Support
  #
  MdeModulePkg/Universal/Acpi/BootGraphicsResourceTableDxe/BootGraphicsResourceTableDxe.inf
  MdeModulePkg/Universal/Acpi/AcpiPlatformDxe/AcpiPlatformDxe.inf
  MdeModulePkg/Universal/Acpi/AcpiTableDxe/AcpiTableDxe.inf
  QemuSbsaPkg/AcpiTables/AcpiTables.inf
  QemuSbsaPkg/SbsaQemuAcpiDxe/SbsaQemuAcpiDxe.inf

  #
  # Standalone MM drivers in non-secure world
  #
  ArmPkg/Drivers/MmCommunicationDxe/MmCommunication.inf {
    <LibraryClasses>
      NULL|StandaloneMmPkg/Library/VariableMmDependency/VariableMmDependency.inf
  }

  #
  # EBC support
  #
  MdeModulePkg/Universal/EbcDxe/EbcDxe.inf

  #
  # Standalone MM modules in secure world
  #
  StandaloneMmPkg/Core/StandaloneMmCore.inf {
    <PcdsFixedAtBuild>
      gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x80000000
  }

  ArmPkg/Drivers/StandaloneMmCpu/StandaloneMmCpu.inf {
    <PcdsFixedAtBuild>
      gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x80000000
  }
  MdeModulePkg/Universal/FaultTolerantWriteDxe/FaultTolerantWriteStandaloneMm.inf
  MdeModulePkg/Universal/Variable/RuntimeDxe/VariableStandaloneMm.inf {
    <LibraryClasses>
      DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
      NULL|MdeModulePkg/Library/VarCheckUefiLib/VarCheckUefiLib.inf
      NULL|MdeModulePkg/Library/VarCheckPolicyLib/VarCheckPolicyLibStandaloneMm.inf
      BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
      VariablePolicyLib|MdeModulePkg/Library/VariablePolicyLib/VariablePolicyLib.inf
      VariablePolicyHelperLib|MdeModulePkg/Library/VariablePolicyHelperLib/VariablePolicyHelperLib.inf
  }
  ArmPlatformPkg/Drivers/NorFlashDxe/NorFlashStandaloneMm.inf

###################################################################################################
#
# BuildOptions Section - Define the module specific tool chain flags that should be used as
#                        the default flags for a module. These flags are appended to any
#                        standard flags that are defined by the build process. They can be
#                        applied for any modules or only those modules with the specific
#                        module style (EDK or EDKII) specified in [Components] section.
#
###################################################################################################
[BuildOptions]
  RVCT:RELEASE_*_*_CC_FLAGS  = -DMDEPKG_NDEBUG

  GCC:RELEASE_*_*_CC_FLAGS  = -DMDEPKG_NDEBUG

  #
  # Disable deprecated APIs.
  #
  RVCT:*_*_*_CC_FLAGS = -DDISABLE_NEW_DEPRECATED_INTERFACES
  GCC:*_*_*_CC_FLAGS = -DDISABLE_NEW_DEPRECATED_INTERFACES

[BuildOptions.common.EDKII.DXE_CORE,BuildOptions.common.EDKII.DXE_DRIVER,BuildOptions.common.EDKII.UEFI_DRIVER,BuildOptions.common.EDKII.UEFI_APPLICATION,BuildOptions.common.EDKII.MM_CORE_STANDALONE,BuildOptions.common.EDKII.MM_STANDALONE]
  GCC:*_*_*_DLINK_FLAGS = -z common-page-size=0x1000

[BuildOptions.common.EDKII.DXE_RUNTIME_DRIVER]
  GCC:*_*_ARM_DLINK_FLAGS = -z common-page-size=0x1000
  GCC:*_*_AARCH64_DLINK_FLAGS = -z common-page-size=0x10000
  RVCT:*_*_ARM_DLINK_FLAGS = --scatter $(EDK_TOOLS_PATH)/Scripts/Rvct-Align4K.sct

[BuildOptions.AARCH64.EDKII.MM_CORE_STANDALONE,BuildOptions.AARCH64.EDKII.MM_STANDALONE]
  GCC:*_*_*_CC_FLAGS = -mstrict-align -march=armv8-a
