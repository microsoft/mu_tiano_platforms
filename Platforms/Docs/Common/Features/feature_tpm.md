# TPM Emulation

For more information on QEMU tpm, see the [QEMU TPM Documentation](https://www.qemu.org/docs/master/specs/tpm.html#the-qemu-tpm-emulator-device).

The QEMU TPM relies on a seperate program to emulate the TPM. Currently, this is
only supported on Linux using the [swtpm program](https://github.com/stefanberger/swtpm).
Swtpm can be installed from the linux package managers. Note that Swtpm is installed by
default in the Docker container. It is recommended to use the Docker container whenever
possible such that software versioning matches that of CI during development.

```bash
sudo apt-get install swtpm
```

To run using this TPM, build and run with the following options. `SWTPM_ENABLE`
enables the swtpm emulator that is started automatically by `QemuRunner.py`.
`SWTPM_ENABLE` is `TRUE` by default.

for the Q35 platform:

```bash
stuart_build -c Platforms/QemuQ35Pkg/PlatformBuild.py --flashrom TOOL_CHAIN_TAG=GCC5 BLD_*_TPM2_ENABLE=TRUE
```

for the Arm Virt platform:

```bash
stuart_build -c Platforms/QemuArmVirtPkg/PlatformBuild.py --flashrom TOOL_CHAIN_TAG=GCC5 BLD_*_TPM2_ENABLE=TRUE
```
