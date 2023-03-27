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
  DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  PeiServicesLib|MdePkg/Library/PeiServicesLib/PeiServicesLib.inf
  RegisterFilterLib|MdePkg/Library/RegisterFilterLibNull/RegisterFilterLibNull.inf

  # Libraries used for test modules.
  UnitTestLib|UnitTestFrameworkPkg/Library/UnitTestLib/UnitTestLib.inf
  UnitTestPersistenceLib|UnitTestFrameworkPkg/Library/UnitTestPersistenceLibNull/UnitTestPersistenceLibNull.inf
  UnitTestResultReportLib|UnitTestFrameworkPkg/Library/UnitTestResultReportLib/UnitTestResultReportLibDebugLib.inf

[LibraryClasses.common]
  BaseBinSecurityLib|MdePkg/Library/BaseBinSecurityLibNull/BaseBinSecurityLibNull.inf

!if $(TOOL_CHAIN_TAG) == VS2019 or $(TOOL_CHAIN_TAG) == VS2022
[LibraryClasses.X64]
  # Provide StackCookie support lib so that we can link to /GS exports for VS builds
  RngLib|MdePkg/Library/BaseRngLib/BaseRngLib.inf
  BaseBinSecurityLib|MdePkg/Library/BaseBinSecurityLibRng/BaseBinSecurityLibRng.inf
  NULL|MdePkg/Library/BaseBinSecurityLibRng/BaseBinSecurityLibRng.inf
!endif

[LibraryClasses.ARM, LibraryClasses.AARCH64]
  NULL|MdePkg/Library/CompilerIntrinsicsLib/ArmCompilerIntrinsicsLib.inf
  NULL|MdePkg/Library/BaseStackCheckLib/BaseStackCheckLib.inf

[LibraryClasses.common.PEIM]
  MemoryAllocationLib|MdePkg/Library/PeiMemoryAllocationLib/PeiMemoryAllocationLib.inf
  HobLib|MdePkg/Library/PeiHobLib/PeiHobLib.inf
  PeimEntryPoint|MdePkg/Library/PeimEntryPoint/PeimEntryPoint.inf
  PeiServicesTablePointerLib|MdePkg/Library/PeiServicesTablePointerLib/PeiServicesTablePointerLib.inf

[LibraryClasses.common.DXE_DRIVER]
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  UefiDriverEntryPoint|MdePkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.inf
  HobLib|MdePkg/Library/DxeHobLib/DxeHobLib.inf

[Components]
  QemuPkg/Library/BasePciCapLib/BasePciCapLib.inf
  QemuPkg/Library/BasePciCapPciSegmentLib/BasePciCapPciSegmentLib.inf
  QemuPkg/Library/ConfigSystemModeLibQemu/ConfigSystemModeLib.inf
  QemuPkg/Library/LockBoxLib/LockBoxBaseLib.inf
  QemuPkg/Library/LockBoxLib/LockBoxDxeLib.inf
  QemuPkg/Library/MsBootOptionsLibQemu/MsBootOptionsLib.inf
  QemuPkg/Library/PlatformBmPrintScLib/PlatformBmPrintScLib.inf
  QemuPkg/Library/PlatformSecureLib/PlatformSecureLib.inf
  QemuPkg/Library/PlatformThemeLib/PlatformThemeLib.inf
  QemuPkg/Library/QemuBootOrderLib/QemuBootOrderLib.inf
  QemuPkg/Library/Tcg2PhysicalPresenceLibNull/DxeTcg2PhysicalPresenceLib.inf
  QemuPkg/Library/Tcg2PhysicalPresenceLibQemu/DxeTcg2PhysicalPresenceLib.inf
  QemuPkg/Library/UefiPciCapPciIoLib/UefiPciCapPciIoLib.inf
  QemuPkg/Library/VirtioLib/VirtioLib.inf
  QemuPkg/Library/XenPlatformLib/XenPlatformLib.inf
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
