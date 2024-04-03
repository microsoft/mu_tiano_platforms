## @file
# Common QEMU modules.
#
# Copyright (C) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: BSD-2-Clause-Patent
##

[Defines]
  PLATFORM_NAME                  = QemuPkg
  PLATFORM_GUID                  = 1BD7DEBC-1571-45E7-9C2D-4AF508E59296
  PLATFORM_VERSION               = 1.0
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/QemuPkg
  SUPPORTED_ARCHITECTURES        = IA32|X64|EBC|ARM|AARCH64|RISCV64
  BUILD_TARGETS                  = DEBUG|RELEASE|NOOPT
  SKUID_IDENTIFIER               = DEFAULT

[LibraryClasses.common]
  DebugLib                     |MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  BaseLib                      |MdePkg/Library/BaseLib/BaseLib.inf
  BaseMemoryLib                |MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  MemoryAllocationLib          |MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  UefiLib                      |MdePkg/Library/UefiLib/UefiLib.inf
  PrintLib                     |MdePkg/Library/BasePrintLib/BasePrintLib.inf
  TimerLib                     |MdePkg/Library/BaseTimerLibNullTemplate/BaseTimerLibNullTemplate.inf
  PcdLib                       |MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  DevicePathLib                |MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  UefiRuntimeServicesTableLib  |MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  PeiServicesLib               |MdePkg/Library/PeiServicesLib/PeiServicesLib.inf
  HiiLib                       |MdeModulePkg/Library/UefiHiiLib/UefiHiiLib.inf
  NULL                         |MdePkg/Library/StackCheckLibNull/StackCheckLibNull.inf

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

  # Libraries used for test modules.
  UnitTestLib|UnitTestFrameworkPkg/Library/UnitTestLib/UnitTestLib.inf
  UnitTestPersistenceLib|UnitTestFrameworkPkg/Library/UnitTestPersistenceLibNull/UnitTestPersistenceLibNull.inf
  UnitTestResultReportLib|UnitTestFrameworkPkg/Library/UnitTestResultReportLib/UnitTestResultReportLibDebugLib.inf

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

  # Sorter helper Libraries
  SortLib              |MdeModulePkg/Library/UefiSortLib/UefiSortLib.inf
  OrderedCollectionLib |MdePkg/Library/BaseOrderedCollectionRedBlackTreeLib/BaseOrderedCollectionRedBlackTreeLib.inf

  # Shell Libraries
  ShellLib         |ShellPkg/Library/UefiShellLib/UefiShellLib.inf
  ShellCommandLib  |ShellPkg/Library/UefiShellCommandLib/UefiShellCommandLib.inf
  ShellCEntryLib   |ShellPkg/Library/UefiShellCEntryLib/UefiShellCEntryLib.inf
  HandleParsingLib |ShellPkg/Library/UefiHandleParsingLib/UefiHandleParsingLib.inf
  BcfgCommandLib   |ShellPkg/Library/UefiShellBcfgCommandLib/UefiShellBcfgCommandLib.inf

  # File Libraries
  FileExplorerLib   |MdeModulePkg/Library/FileExplorerLib/FileExplorerLib.inf
  FileHandleLib     |MdePkg/Library/UefiFileHandleLib/UefiFileHandleLib.inf
  UefiDecompressLib |MdePkg/Library/BaseUefiDecompressLib/BaseUefiDecompressLib.inf

  # TPM Libraries
  OemTpm2InitLib          |SecurityPkg/Library/OemTpm2InitLibNull/OemTpm2InitLib.inf
  Tpm2DebugLib            |SecurityPkg/Library/Tpm2DebugLib/Tpm2DebugLibNull.inf
  Tpm12CommandLib         |SecurityPkg/Library/Tpm12CommandLib/Tpm12CommandLib.inf
  Tpm2CommandLib          |SecurityPkg/Library/Tpm2CommandLib/Tpm2CommandLib.inf
  Tpm2DeviceLib           |SecurityPkg/Library/Tpm2DeviceLibTcg2/Tpm2DeviceLibTcg2.inf
  Tcg2PpVendorLib         |SecurityPkg/Library/Tcg2PpVendorLibNull/Tcg2PpVendorLibNull.inf
  TpmMeasurementLib       |MdeModulePkg/Library/TpmMeasurementLibNull/TpmMeasurementLibNull.inf
  Tcg2PhysicalPresenceLib |QemuPkg/Library/Tcg2PhysicalPresenceLibNull/DxeTcg2PhysicalPresenceLib.inf

[LibraryClasses.common.PEIM]
  Tpm12DeviceLib             |SecurityPkg/Library/Tpm12DeviceLibDTpm/Tpm12DeviceLibDTpm.inf
  Tpm2DeviceLib              |SecurityPkg/Library/Tpm2DeviceLibDTpm/Tpm2DeviceLibDTpm.inf
  SourceDebugEnabledLib      |SourceLevelDebugPkg/Library/SourceDebugEnabled/SourceDebugEnabledLib.inf

[LibraryClasses.ARM, LibraryClasses.AARCH64]
  NULL|MdePkg/Library/CompilerIntrinsicsLib/ArmCompilerIntrinsicsLib.inf
  NULL|MdePkg/Library/BaseStackCheckLib/BaseStackCheckLib.inf
  IoLib|MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsicArmVirt.inf

[LibraryClasses.common.PEIM]
  MemoryAllocationLib|MdePkg/Library/PeiMemoryAllocationLib/PeiMemoryAllocationLib.inf
  HobLib|MdePkg/Library/PeiHobLib/PeiHobLib.inf
  PeimEntryPoint|MdePkg/Library/PeimEntryPoint/PeimEntryPoint.inf
  PeiServicesTablePointerLib|MdePkg/Library/PeiServicesTablePointerLib/PeiServicesTablePointerLib.inf

[LibraryClasses.common.DXE_DRIVER]
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  UefiDriverEntryPoint|MdePkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.inf
  HobLib|MdePkg/Library/DxeHobLib/DxeHobLib.inf

[PcdsDynamicDefault.common]
  #
  # TPM2 support
  #
  gEfiSecurityPkgTokenSpaceGuid.PcdTpmBaseAddress|0x0
  gEfiSecurityPkgTokenSpaceGuid.PcdTpmInstanceGuid|{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
  gEfiSecurityPkgTokenSpaceGuid.PcdTpm2HashMask|0

[Components]
  QemuPkg/Library/BaseFwCfgInputChannelLib/BaseFwCfgInputChannelLib.inf
  QemuPkg/Library/BasePciCapLib/BasePciCapLib.inf
  QemuPkg/Library/BasePciCapPciSegmentLib/BasePciCapPciSegmentLib.inf
  QemuPkg/Library/ConfigSystemModeLibQemu/ConfigSystemModeLib.inf
  QemuPkg/Library/DfciDeviceIdSupportLib/DfciDeviceIdSupportLib.inf
  QemuPkg/Library/DfciUiSupportLib/DfciUiSupportLib.inf
  QemuPkg/Library/LockBoxLib/LockBoxBaseLib.inf
  QemuPkg/Library/LockBoxLib/LockBoxDxeLib.inf
  QemuPkg/Library/MsBootOptionsLibQemu/MsBootOptionsLib.inf
  QemuPkg/Library/PlatformBmPrintScLib/PlatformBmPrintScLib.inf
  QemuPkg/Library/PlatformSecureLib/PlatformSecureLib.inf
  QemuPkg/Library/PlatformThemeLib/PlatformThemeLib.inf
  QemuPkg/Library/Tcg2PhysicalPresenceLibNull/DxeTcg2PhysicalPresenceLib.inf
  QemuPkg/Library/Tcg2PhysicalPresenceLibQemu/DxeTcg2PhysicalPresenceLib.inf
  QemuPkg/Library/UefiPciCapPciIoLib/UefiPciCapPciIoLib.inf
  QemuPkg/Library/VirtioLib/VirtioLib.inf
  QemuPkg/Library/QemuFwCfgLib/QemuFwCfgLibNull.inf
  QemuPkg/Library/QemuPreUefiEventLogLibNull/QemuPreUefiEventLogLibNull.inf
  QemuPkg/Library/XenPlatformLib/XenPlatformLib.inf
  QemuPkg/FrontPageButtons/FrontPageButtons.inf
  QemuPkg/PciHotPlugInitDxe/PciHotPlugInit.inf
  QemuPkg/VirtioPciDeviceDxe/VirtioPciDeviceDxe.inf
  QemuPkg/Virtio10Dxe/Virtio10.inf
  QemuPkg/VirtioBlkDxe/VirtioBlk.inf
  QemuPkg/VirtioScsiDxe/VirtioScsi.inf
  QemuPkg/VirtioRngDxe/VirtioRng.inf
  QemuPkg/VirtioNetDxe/VirtioNet.inf
  QemuPkg/SataControllerDxe/SataControllerDxe.inf
  QemuPkg/LinuxInitrdDynamicShellCommand/LinuxInitrdDynamicShellCommand.inf
  QemuPkg/Tcg/Tcg2Config/Tcg12ConfigPei.inf
  QemuPkg/Tcg/Tcg2Config/Tcg2ConfigPei.inf
