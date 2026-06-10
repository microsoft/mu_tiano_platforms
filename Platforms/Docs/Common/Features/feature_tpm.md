# TPM Emulation

For more information on QEMU tpm, see the [QEMU TPM Documentation](https://www.qemu.org/docs/master/specs/tpm.html#the-qemu-tpm-emulator-device).

The QEMU TPM relies on a seperate program to emulate the TPM. Currently, this is
only supported on Linux using the [swtpm program](https://github.com/stefanberger/swtpm).
Swtpm can be installed from the linux package managers.

```bash
sudo apt-get install swtpm
```

To run using this TPM, build and run with the following options. `SWTPM_ENABLE`
enables the swtpm emulator that is started automatically by `QemuRunner.py`.
`SWTPM_ENABLE` is `TRUE` by default.

for the Q35 platform:
```bash
stuart_build -c Platforms/QemuQ35Pkg/PlatformBuild.py --flashrom TOOL_CHAIN_TAG=GCC5 BLD_*_TPM_ENABLE=TRUE
```

for the Arm Virt platform:
```bash
stuart_build -c Platforms/QemuArmVirtPkg/PlatformBuild.py --flashrom TOOL_CHAIN_TAG=GCC5 BLD_*_TPM2_ENABLE=TRUE
```

In the window running swtpm, there should be output from the TPM communication.

```txt
Ctrl Cmd: length 4
00 00 00 10
Ctrl Rsp: length 4
00 00 00 00
SWTPM_IO_Read: length 10
80 01 00 00 00 0A 00 00 01 81
SWTPM_IO_Write: length 10
80 01 00 00 00 0A 00 00 01 01
...
```
