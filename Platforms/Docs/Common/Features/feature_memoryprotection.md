# Memory Protection

For in-depth information on Memory Protection in Project Mu, see
[feature_memory_protection.md](../../../../MU_BASECORE/Docs/feature_memory_protection.md)

Memory protection is ON by default on Q35 and SBSA. To disable memory protection
add `BLD_*_MEMORY_PROTECTION=FALSE` to your `stuart_build` command. Example:

`stuart_build -c .\Platforms\<Platform>\PlatformBuild.py BLD_*_MEMORY_PROTECTION=FALSE --FlashRom`

Because `MEMORY_PROTECTION` is a build flag, the platform will need to be rebuilt for a change to the
value to take effect (meaning `--FlashOnly` will not work).
