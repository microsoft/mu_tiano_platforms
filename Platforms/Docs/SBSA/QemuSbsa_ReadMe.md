# QemuSbsaPkg

**QemuSbsaPkg...**

- Is another derivative of OvmfPkg based on EDK2 QEMU-SBSA ARM machine type.
- Will not support Legacy BIOS or CSM.
- WIll not support S3 sleep functionality.
- Has 64-bit for both PEI and DXE phase.
- Seeks to enable a tightly constrained virtual platform based on the QEMU ARM CPUs.

By solely focusing on the ARM chipset, this package can be optimized such that it is allowed to break compatibility
with other QEMU supported chipsets. The ARM chipset can be paired with an AARCH64 processor to enable a machine
that can emulate ARM based hardware with industry standard features like TrustZone and PCI-E. Although leveraging
SBSA machine type provided by QEMU, the features enabled/included in this package will not be server class platform
centric.

## QEMU-SBSA Platform

SBSA is an ARM based machine type that QEMU emulates.

The advantages of the SBSA over the virtual ARM platform (which is what QEMU often emulates) is that it has
better ARM based platform level support (ACPI, etc.) as well as having an integrated AHCI controller.

## Compiling and Running QEMU

QemuSbsaPkg uses the Project Mu repositories and Edk2 PyTools for its build operations.
Specific details can be found here [Development/building.md](../Common/building.md)

## Firmware Features

QemuSbsaPkg is a great environment to demonstrate Project Mu features without any restricted or costly physical
hardware. Current QEMU SBSA platform supports the following features provided by Project Mu:

### Mu Front Page

Enable the Project Mu OEM sample "front page".
This is a touch friendly, graphical, UEFI HII based UI application that
allows basic platform and boot device configuration.

[Details](../Common/Features/feature_frontpage.md)

### Device Firmware Configuration Interface

The DFCI feature enables cloud management services (MDM services like Microsoft Intune) to manage some PC
bios settings **securely**.  DFCI is a foundational feature that provides a shared identity and ownership
model between the device firmware and the cloud.  Once a device is enrolled this shared identity can be used
to securely communicate across untrusted mediums (network or usb).

[Details](../Common/Features/feature_dfci.md)

### Mu Telemetry / WHEA / HwErrorRecord

The Mu Telemetry feature is an extension of the PI spec defined report status code.  The feature is
designed to collect critical (platform defined) status codes, record them into a HwErrRecord,
and then transfer them through the Microsoft WHEA pipeline.  From there an OEM can use Microsoft
provided reports to check on in market device health.  *Some work still pending completion.

[Details](../Common/Features/feature_whea.md)

### Trusted Platform Module (TPM)

QEMU TPM emulation implements a TPM TIS hardware interface that follows the Trusted Computing Group's TCG PC Client
Specific TPM Interface Specification (TIS) in addition to a TPM CRB interface that follows the TCG PC Client Platform
TPM Profile (PTP) Specification. `QemuSbsaPkg` has support to include TPM drivers and connect to the software TPM
socket interface. Usage is covered in the detailed feature readme.

[Details](../Common/Features/feature_tpm.md)

_Note_: QEMU TPM Emulation requires the change in QEMU source code to enable TPM support:

```diff
diff --git a/hw/arm/sbsa-ref.c b/hw/arm/sbsa-ref.c
index e3195d5449..84bc7d9adb 100644
--- a/hw/arm/sbsa-ref.c
+++ b/hw/arm/sbsa-ref.c
@@ -28,6 +28,8 @@
 #include "sysemu/numa.h"
 #include "sysemu/runstate.h"
 #include "sysemu/sysemu.h"
+#include "sysemu/tpm.h"
+#include "sysemu/tpm_backend.h"
 #include "exec/hwaddr.h"
 #include "kvm_arm.h"
 #include "hw/arm/boot.h"
@@ -94,6 +96,7 @@ enum {
     SBSA_SECURE_MEM,
     SBSA_AHCI,
     SBSA_XHCI,
+    SBSA_TPM,
 };
 
 struct SBSAMachineState {
@@ -132,6 +135,7 @@ static const MemMapEntry sbsa_ref_memmap[] = {
     /* Space here reserved for more SMMUs */
     [SBSA_AHCI] =               { 0x60100000, 0x00010000 },
     [SBSA_XHCI] =               { 0x60110000, 0x00010000 },
+    [SBSA_TPM] =                { 0x60120000, 0x00010000 },
     /* Space here reserved for other devices */
     [SBSA_PCIE_PIO] =           { 0x7fff0000, 0x00010000 },
     /* 32-bit address PCIE MMIO space */
@@ -629,6 +633,24 @@ static void create_smmu(const SBSAMachineState *sms, PCIBus *bus)
     }
 }
 
+static void create_tpm(SBSAMachineState *sbsa, PCIBus *bus)
+{
+    Error *errp = NULL;
+    DeviceState *dev;
+
+    TPMBackend *be = qemu_find_tpm_be("tpm0");
+    if (be == NULL) {
+        error_report("Couldn't find tmp0 backend");
+        return;
+    }
+
+    dev = qdev_new(TYPE_TPM_TIS_SYSBUS);
+    object_property_set_link(OBJECT(dev), "tpmdev", OBJECT(be), &errp);
+    object_property_set_str(OBJECT(dev), "tpmdev", be->id, &errp);
+    sysbus_realize_and_unref(SYS_BUS_DEVICE(dev), &error_fatal);
+    sysbus_mmio_map(SYS_BUS_DEVICE(dev), 0, sbsa_ref_memmap[SBSA_TPM].base);
+}
+
 static void create_pcie(SBSAMachineState *sms)
 {
     hwaddr base_ecam = sbsa_ref_memmap[SBSA_PCIE_ECAM].base;
@@ -686,6 +708,8 @@ static void create_pcie(SBSAMachineState *sms)
     pci_create_simple(pci->bus, -1, "bochs-display");
 
     create_smmu(sms, pci->bus);
+
+    create_tpm(sms, pci->bus);
 }
 
 static void *sbsa_ref_dtb(const struct arm_boot_info *binfo, int *fdt_size)
```

## Mu Customized Components

### Modules

| Modules | Link to Documentation |
| --- | --- |
| **QemuVideoDxe** | [QEMU Cirrus Video Controller](../../QemuSbsaPkg/QemuVideoDxe/ReadMe.md) |

### Libraries

| Libraries | Link to Documentation |
| --- | --- |
| **MsPlatformDevicesLib** | [MsPlatformDevicesLib](../../QemuSbsaPkg/Library/MsPlatformDevicesLibQemuSbsa/ReadMe.md) |
