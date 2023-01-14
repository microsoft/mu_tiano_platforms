# UEFI Configuration Platform Guide

The UEFI configuration is a Project MU feature intended to streamline the injection of configuration with better scalability.

## Configuration Logistics

This sections describes the design logistics on how the platforms should integrate configuration framework into the features
they desire to configure.

### Overview

A platform could author as many as hundreds of features to be configured/changed for various purposes. For example, silicon
drivers could configure the number of USB ports to overdrive the current, the index of PCIe ports to be powered, etc. These
flexibilities could be used for hardware validation for silicon features and/or exposed to end users/admins to configure
the system to comply with various marketing/regulatory needs.

However, the silicon validation features are not (always) identical to the configuration knobs exposed to the end users.
For example, the chipset could contain 4 USB controllers and all of them are configurable through silicon validation phase.
But on a given platform, only the first 2 controllers can be configured by the end user because only these 2 controllers
have physical outlets. This is where the differences between __policy__ and __configuration__ start to emerge.

### Policy Data

Policy, in this context, refers to the [PolicyServicePkg](https://microsoft.github.io/mu/dyn/mu_basecore/PolicyServicePkg/)
in [MU_BASECORE](https://github.com/microsoft/mu_basecore). This framework creates volatile GUIDed database during boot
time and supports notification at data creation time. This data structure is defined by the feature module and should be
published during early boot time with initial values desirable for the platform to boot properly. This data should also be
consumed by the feature module to configure the hardware or other entities as needed.

### Configuration Data

Configuration, in contrast, is defined by the platform with their discretion. This means that the end design of configuration
knobs is not 1:1 mapped into the corresponding policy data. Thus the platform could selectively expose a few configuration
knobs from policy data structure and allow the end user/admin to configure through configuration framework. Or alternatively,
coalesce a group of policy switches into a single configuration knob.

In this case, the platform module is responsible for consuming the injected configuration data (in the format of UFEI variables)
then translating and applying the change on top of existing policy data so the feature module can consume the modified policy
data according to configuration data.

### Configuration Data Delivery

A more detailed description of how configuration data is delivered to UEFI variable storage is described in the feature repo
[here](https://github.com/microsoft/mu_feature_config/blob/main/SetupDataPkg/Docs/Overview/Overview.md).

## Feature Integration Guidance

This section describes the workflow, from the platform feature owner's perspective, on how to integrate the configuration
framework into a single feature. To simplify the language, this flow uses the GFX configuration knob as an example to explain
the expected workflow.

### Author/Modify the Final Consumer Module Around Policy Package

For a feature that needs to be configured through this framework, the end module should be updated to consume the data
described in [PolicyServicePkg](https://microsoft.github.io/mu/dyn/mu_basecore/PolicyServicePkg/).

In the example of GFX module, [QemuQ35Pkg/QemuVideoDxe/QemuVideoDxe.inf](../../QemuVideoDxe/QemuVideoDxe.inf) is updated
to check against the published policy data (`GFX_POLICY_DATA`) to enable/disable the applicable VGA controller during the
driver binding event.

### Author the Initial Policy Publisher

As mentioned above, given the GFX module is updated to consume the policy data, a module is needed to publish the initial
values for the feature module to proceed properly. Per platform discretion, this module could either be a silicon module
if a general default policy data is applicable, or a platform module when the platform would like to apply customizations
to the inital policy value.

In the example of GFX module, this initial value is handled by [QemuQ35Pkg/ConfigDataGfx/ConfigDataGfx.inf](../../ConfigDataGfx/ConfigDataGfx.inf),
which publishes a policy blob under GUID `gPolicyDataGFXGuid` to enable all GFX controllers by default.

### Author the Platform Configuration Consumer

Once the initial policy data is published, the platform driver should check the configuration data, as defined by platform
configuratuin definition files (i.e. XML) to translate the data and apply them on top of the GFX policy data.

In the example of GFX module, this translation and data application is also handled by [QemuQ35Pkg/ConfigDataGfx/ConfigDataGfx.inf](../../ConfigDataGfx/ConfigDataGfx.inf).

Note that with the support of GUIDed policy database and notification, the platform configuration consumer does not always
need to be the same driver as the policy publisher, but can instead be a module with a Depex on the published policy data
GUID (in this case, `gPolicyDataGFXGuid`).

This way, when [QemuQ35Pkg/QemuVideoDxe/QemuVideoDxe.inf](../../QemuVideoDxe/QemuVideoDxe.inf) enters the driver binding
event, it will check against the updated policy database and enable/disable the VGA controller accordingly.
