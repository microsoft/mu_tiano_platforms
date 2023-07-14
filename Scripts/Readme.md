# MU Tiano Platforms Scripts

This folder contains scripts useful for using the virtual platforms in this
repository outside of the context of UEFI development.

## MuEmu Script

This script provides useful functionality for acquiring and running the Project
MU QEMU platforms locally without having to build the firmware directly. This
script is intended to be used outside of the repo. An example of this script can
be seen below. For full options use the `--help` argument.

```cmd
python3 muemu.py --update --version latest
python3 muemu.py --arch x64 --disk boot.qcow2
```
