# TPM Emulation

For more information on QEMU tpm, see the [QEMU TPM Documentation](https://www.qemu.org/docs/master/specs/tpm.html#the-qemu-tpm-emulator-device).

The QEMU TPM relies on a seperate program to emulate the TPM. Currently, this is
only supported on Linux using the [swtpm program](https://github.com/stefanberger/swtpm).
Swtpm can be installed from the linux package managers.

```bash
sudo apt-get install swtpm
```

To start the TPM emulator, invoke swtpm with a state file location and character
device.

```bash
mkdir /tmp/mytpm1
swtpm socket --tpmstate dir=/tmp/mytpm1 \
  --ctrl type=unixio,path=/tmp/mytpm1/swtpm-sock \
  --tpm2 \
  --log level=20
```

To run Q35 using this TPM, build and run with the following options. `TPM_DEV` should
point to the path of the character device from the above swtpm command.

```bash
stuart_build -c Platforms/QemuQ35Pkg/PlatformBuild.py --flashrom TOOL_CHAIN_TAG=GCC5 BLD_*_TPM_ENABLE=TRUE TPM_DEV=/tmp/mytpm1/swtpm-sock
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
