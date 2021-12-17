# BranchReadme efi_memory_attributes

This branch is used to showcase, test, and develop production UEFI memory mitigations.  There are numerous
areas for compatibility challenges and thus a large number of loosely connected partners will need to work
together to find an acceptable solution for our shared customers while still improving FW security.

Branched from: release/202102

Branched on: 12/16/2021

Author: sean.brogan@microsoft.com, tabeebe@microsoft.com

## What's unique about this branch

In this branch a new code first/prototype protocol has been enabled on the QemuQ35 platform.
Protocol is actually define here in mu_basecore <https://github.com/microsoft/mu_basecore/blob/poc/efi_mem_attributes/MdePkg/Include/Protocol/MemoryAttribute.h>
Unit test here <https://github.com/microsoft/mu_basecore/tree/poc/efi_mem_attributes/MdePkg/Test/ShellTest/MemoryAttributeProtocolFuncTestApp>
Implemented here <https://github.com/microsoft/mu_basecore/blob/poc/efi_mem_attributes/UefiCpuPkg/CpuDxe/CpuPageTable.c#L1629>

This protocol allows a UEFI compliant loader to change memory attributes on a given memory range.  This will allow
the DXE Core/UEFI platform to tighten module loading and memory allocations (use NX attribute) while still
allowing a boot loaders that load additional modules to easily make the code executable.

## Roadmap

Dec/Jan - 2021/2022 - Prototype and find an industry compatible solution taking into account the last few years
of prevalent pre-boot loaders.

## Expected lifetime of your branch

This work should wrap up by mid 2022 and be merged into release branches/upstream and production platforms.

## Todo

* Update UEFI PE loaders to support all memory allocations marked as NX
* Update BDS, UEFI PE Loader, and memory manager to support a backward compatible solution
* Evaluate adding support in UEFI PE loader for RO data sections
* Validate gaps in the Dxe environment regarding memory mitigations
  * Improve DxePagingAudit <https://github.com/microsoft/mu_plus/tree/release/202102/UefiTestingPkg/AuditTests/PagingAudit>
  * Functional test here <https://github.com/microsoft/mu_plus/tree/release/202102/UefiTestingPkg/FunctionalSystemTests/HeapGuardTest>
