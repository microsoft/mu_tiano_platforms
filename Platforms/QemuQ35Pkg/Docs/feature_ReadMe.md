# Feature List

QemuQ35Pkg Feature List

## Mu Front Page

Enable the Project Mu OEM sample "front page".
This is a touch friendly, graphical, UEFI HII based UI application that
allows basic platform and boot device configuration.

[Details](feature_frontpage.md)

## Device Firmware Configuration Interface

The DFCI feature enables cloud management services (MDM services like Microsoft Intune) to manage some PC
bios settings **securely**.  DFCI is a foundational feature that provides a shared identity and ownership
model between the device firmware and the cloud.  Once a device is enrolled this shared identity can be used
to securely communicate across untrusted mediums (network or usb).

[Details](feature_dfci.md)

## Mu Telemetry / WHEA / HwErrorRecord

The Mu Telemetry feature is an extension of the PI spec defined report status code.  The feature is
designed to collect critical (platform defined) status codes, record them into a HwErrRecord,
and then transfer them through the Microsoft WHEA pipeline.  From there an OEM can use Microsoft
provided reports to check on in market device health.  *Some work still pending completion.

[Details](feature_whea.md)
