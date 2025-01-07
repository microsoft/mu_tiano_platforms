# Secure Partition & Service Guide

## Secure Partitions Overview

Secure partitions are isolated environments which are used to instantiate management or security services
in a trusted environment. They are managed by a Secure Partition Manager (SPM) which ensures that each
partition operates independently and securely. A secure partition can have multiple services which it
manages. Secure partitions can be set to run at S-EL0 or S-EL1 with S-EL1 providing higher privileges.

## Secure Partitions Implementation

The following are the steps needed to create a secure partition:

1. Create a new folder for your secure partition in the Platforms/QemuSbsaPkg directory making sure to give it a relevant
   name.
2. Create the two necessary files: a .c which will contain the implementation for the secure partition and a .inf which
   will be used to launch the secure partition similar to a DXE driver.
3. Generate a UUID/GUID for your secure partition, this will distinguish it from other secure partitions.
4. Add the .inf to the QemuSbsaPkg.dsc and .fdf files. If overriding the MsSecurePartition, just replace all instances
   with your secure partition. If wanting to add another secure partition, you will need to add new sections to the .fdf
   and PlatformBuild.py script to generate a new FV for your secure partition similar to [\FV.FV_STANDALONE_MM_SECURE_PARTITION1].

   .dsc example:

   ```c
   QemuSbsaPkg/MsSecurePartition/MsSecurePartition.inf {
    <LibraryClasses>
      MemoryAllocationLib|MdeModulePkg/Library/BaseMemoryAllocationLibNull/BaseMemoryAllocationLibNull.inf
      StandaloneMmCoreEntryPoint|ArmPkg/Library/SecurePartitionEntryPoint/SecurePartitionEntryPoint.inf
      Tpm2DeviceLib|SecurityPkg/Library/Tpm2DeviceLibDTpm/Tpm2DeviceLibDTpmStandaloneMm.inf
      Tpm2DebugLib|SecurityPkg/Library/Tpm2DebugLib/Tpm2DebugLibVerbose.inf
      TimerLib|ArmPkg/Library/ArmArchTimerLibEx/ArmArchTimerLibEx.inf
    <PcdsFixedAtBuild>
      gEfiMdeModulePkgTokenSpaceGuid.PcdSerialRegisterBase|0x60030000
      gEfiSecurityPkgTokenSpaceGuid.PcdTpmBaseAddress|0x60120000
      gEfiSecurityPkgTokenSpaceGuid.PcdTpmInternalBaseAddress|0x10000200000
      gArmTokenSpaceGuid.PcdArmArchTimerFreqInHz|62500000
    <PcdsPatchableInModule>
      gArmTokenSpaceGuid.PcdFfaLibConduitSmc|FALSE
   }
   ```

   .fdf example:

   ```c
   [FV.FV_STANDALONE_MM_SECURE_PARTITION1]
   FvAlignment        = 16
   ERASE_POLARITY     = 1
   MEMORY_MAPPED      = TRUE
   STICKY_WRITE       = TRUE
   LOCK_CAP           = TRUE
   LOCK_STATUS        = TRUE
   WRITE_DISABLED_CAP = TRUE
   WRITE_ENABLED_CAP  = TRUE
   WRITE_STATUS       = TRUE
   WRITE_LOCK_CAP     = TRUE
   WRITE_LOCK_STATUS  = TRUE
   READ_DISABLED_CAP  = TRUE
   READ_ENABLED_CAP   = TRUE
   READ_STATUS        = TRUE
   READ_LOCK_CAP      = TRUE
   READ_LOCK_STATUS   = TRUE

    INF QemuSbsaPkg/MsSecurePartition/MsSecurePartition.inf
   ```

5. Create the .dts file for your secure partition and place it in the Platforms/QemuSbsa/Pkg/fdts directory. If overriding
   the MsSecurePartition, the qemu_sbsa_mssp_config.dts can be updated with the settings related to your secure partition
   Note that only S-EL0 partitions are supported at this time.
6. Update the .dts in TFA to include your secure partition and its info. The file can be found at Silicon/Arm/TFA/plat/qemu/qemu_sbsa/fdts.

   ```c
   secure-partitions {
    compatible = "arm,sp";
    stmm {
      uuid = "eaba83d8-baaf-4eaf-8144-f7fdcbe544a7";
      load-address = <0x20002000>;
      owner = "Plat";
    };
    mssp {
      uuid = "b8bcbd0c-8e8f-4ebe-99eb-3cbbdd0cd412";
      load-address = <0x20400000>;
      owner = "Plat";
    };
   };
   ```

7. Build and run the firmware, you should see Hafnium dispatching your secure partition.

## Services Overview

A service which runs in a secure partition is designed to provide specific functionality similar to a
library. A service will have a UUID associated with it which is used to distinguish it from other
services that may be running within a secure partition. In FF-A, a DIRECT_REQ2 message will contain
the service UUID that the message is meant for in the ARG2 and ARG3 registers. This is how the secure
partition is able to figure out which service to pass along the message to.

## Services Implementation

The following are steps needed to create a service for a secure partition:

1. Create a new folder for your service in the Path/To/Pkg/Library directory making sure to give it a
   relevant name.
2. Create the two necessary files: a .c which will contain the implementation for the service and a .inf which will be
   used to link in your service library to your secure partition.
3. Generate a UUID/GUID for your service, this will be used by FF-A DIRECT_REQ2 to route the message correctly to the
   secure partition and the proper service.
4. Place this UUID/GUID in a .h file which will be added to the Path/To/Pkg/Include/Guid directory. This file should also
   contain the OPCODES your service will support.
5. Create a global EFI_GUID extern which will hold the UUID/GUID of your service. Make sure it is initialized in the .dec
   file. This will give you an easy way to compare the UUID/GUID against other UUIDs/GUIDs received through FF-A.

   ```c
   ## TPM Service over FF-A
   # Include/Guid/Tpm2ServiceFfa.h
   gEfiTpm2ServiceFfaGuid = { 0x17b862a4, 0x1806, 0x4faf, { 0x86, 0xb3, 0x08, 0x9a, 0x58, 0x35, 0x38, 0x61 } }
   ```

6. Create a .h file that will contain all of the global functions related to your service. Place this file in the Path/To/Pkg/Include/Library
   directory. Note that a service should have an Init, Deinit, and Handler function. Make sure to update the .dec to include
   the path to this header.

   ```c
   ##  @libraryclass  Provides an implementation of the TPM Service
   #
   TpmServiceLib|Include/Library/TpmServiceLib.h
   ```

7. Add the UUID/GUID to the uuid variable in the secure partition .dts file. A secure partition can support multiple
   services, each service can be added as a list. (i.e. <\UUID_1>, <\UUID_2>, <\UUID_3>)

   ```c
   uuid = <0xb510b3a3 0x59f64054 0xba7aff2e 0xb1eac765>, <0x17b862a4 0x18064faf 0x86b3089a 0x58353861>, <0xe0fad9b3 0x7f5c42c5 0xb2eeb7a8 0x2313cdb2>;
   ```

8. Link the service into your secure partition by adding the .inf to the [\LibraryClasses] section of the secure partition's
   .inf file.

   ```c
   [LibraryClasses]
    NotificationServiceLib
    TestServiceLib
    TpmServiceLib
   ```

9. In the secure partition .c file, include the headers for the service library. From there you should be able to access
   the Init, Deinit, and handler functions for your service. Note that you will need to extract the UUID from the
   DIRECT_REQ2 message and route it to the correct service. Including the UUID/GUID header from step 4 will allow you to
   use the variable you created in step 5.
