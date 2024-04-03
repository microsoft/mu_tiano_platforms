# Memory Protection

For in-depth technical details on Memory Protection in Project Mu, see
[feature_memory_protection.md](https://github.com/microsoft/mu_basecore/blob/HEAD/Docs/feature_memory_protection.md)

Memory protections are important because Unified Extensible Firmware Interface (UEFI) standard accounts for the
firmware design implemented in 80 to 90 percent of the of PCs and servers sold worldwide. Developed and supported by
more than 250 industry-leading companies, UEFI firmware is responsible for booting and securing billions of devices
spanning device classes from embedded applications to multi-role server systems.

While considerable attention has been devoted to hardware trust anchors and operating system security, attackers have
discovered that UEFI firmware is lacking basic memory protections that have been present in other system software for
over a decade. Coupled with the inconsistency of security capabilities inherit to vendor firmware implementations,
UEFI firmware has become an increasingly attractive system attack vector.

`QemuQ35Pkg` and `QemuSbsaPkg` allow experimentation with the memory protections being offered in physical platforms.

Memory protections are ON by default on Q35 and SBSA. To disable memory protection add `BLD_*_MEMORY_PROTECTION=FALSE`
to your `stuart_build` command. Example:

`stuart_build -c .\Platforms\<Platform>\PlatformBuild.py BLD_*_MEMORY_PROTECTION=FALSE --FlashRom`

Because `MEMORY_PROTECTION` is a build flag, the platform will need to be rebuilt for a change to the value to take
effect (meaning `--FlashOnly` will not work).
