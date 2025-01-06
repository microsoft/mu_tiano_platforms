# Secure Partitions
## Overview
Secure partitions are isolated environments which are used to instantiate management or security services
in a trusted environment. They are managed by a Secure Partition Manager (SPM) which ensures that each
partition operates independently and securely. A secure partition can have multiple services which it
manages. Secure partitions can be set to run at S-EL0 or S-EL1 with S-EL1 providing higher privileges.

## Implementation
The following are the steps needed to create a secure partition:
1. Create a new folder for your secure partition in the Platforms/QemuSbsaPkg directory making sure to give it a relevant name.
2. Create the two necessary files: a .c which will contain the implementation for the secure partition and a .inf which will be
   used to launch the secure partition similar to a DXE driver.
3. Generate a UUID/GUID for your secure partition, this will distinguish it from other secure partitions.
4. Add the .inf to the QemuSbsaPkg.dsc and .fdf files. If overriding the MsSecurePartition, just replace all instances with your
   secure partition. If wanting to add another secure partition, you will need to add new sections to the .fdf and PlatformBuild.py
   script to generate a new FV for your secure partition similar to [\FV.FV_STANDALONE_MM_SECURE_PARTITION1].
5. Create the .dts file for your secure partition and place it in the Platforms/QemuSbsa/Pkg/fdts directory. If overriding the
   MsSecurePartition, the qemu_sbsa_mssp_config.dts can be updated with the settings related to your secure partition. Note that
   only S-EL0 partitions are supported at this time.
6. If adding a new secure partition, update the .dts in TFA to include it. The file can be found at Silicon/Arm/TFA/plat/qemu/qemu_sbsa/fdts.
7. Build and run the firmware, you should see Hafnium dispatching your secure partition. 

# Services
## Overview
A service which runs in a secure partition is designed to provide specific functionality similar to a
library. A service will have a UUID associated with it which is used to distinguish it from other
services that may be running within a secure partition. In FF-A, a DIRECT_REQ2 message will contain
the service UUID that the message is meant for in the ARG2 and ARG3 registers. This is how the secure
partition is able to figure our which service to pass along the message to.

## Implementation
The following are steps needed to create a service for a secure partition:
1. Create a new folder for your servuice in the Silicon/Arm/MU_TIANO/ArmPkg/Library directory making sure to give it a relevant name.
2. Create the two necessary files: a .c which will contain the implementation for the service and a .inf which will be used to link
   in your service library to your secure partition.
3. Generate a UUID/GUID for your service, this will be used by FF-A DIRECT_REQ2 to route the message correctly to the secure partition
   and the proper service.
4. Place this UUID/GUID in a .h file which will be added to the Silicon/Arm/MU_TIANO/ArmPkg/Include/Library directory. This file should
   also contain all of the global functions related to your service. Note that a service should have an Init, Deinit, and Handler function.
   This file will also contain the OPCODES for the functions/commands your service will support.
5. Add the UUID/GUID to the uuid variable in the secure partition .dts file. A secure partition can support multiple services, each service
   can be added as a list. (i.e. <\UUID_1>, <\UUID_2>, <\UUID_3>)
6. Link the service into your secure partition by adding the .inf to the [\LibraryClasses] section of the secure partition's .inf file.
7. In the secure partition .c file, include the headers for the service library. From there you should be able to access the Init, Deinit,
   and handler functions for your service. Note that you will need to extract the UUID from the DIRECT_REQ2 message and route it to the
   correct service.
