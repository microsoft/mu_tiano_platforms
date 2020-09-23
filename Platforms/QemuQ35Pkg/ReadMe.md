# QemuQ35Pkg - Builds and CI

This ReadMe.md describes the Azure DevOps based CI for QemuQ35Pkg and how
to use the same Pytools based build infrastructure locally.  This Qemu platform
is used by Project Mu to develop and share X64 PC class features.

## Supported Configuration Details

This solution for building and running QemuQ35Pkg has only been validated with Windows 10
with VS2019 and Ubuntu 18.04 with GCC5 toolchain.  This solution uses IA32 for PEI and X64
for the DXE phase.  This is done to mimic x64 physical machines. This build supports
different profiles.

| Profile name            | QEMU Features          |
| :----                   | :-----                 |
| default                 |                        |
| net                     |  networking enabled    |

## EDK2 Developer environment

- [Python 3.8.x - Download & Install](https://www.python.org/downloads/)
- [GIT - Download & Install](https://git-scm.com/download/)
- [QEMU - Download, Install, and add to your path](https://www.qemu.org/download/)
- Additional packages found necessary for Ubuntu 18.04
  - apt-get install gcc g++ make uuid-dev qemu

Note: edksetup, Submodule initialization and manual installation of NASM, iASL, or
the required cross-compiler toolchains are **not** required, this is handled by the
Pytools build system.

## Building with Pytools for QemuQ35Pkg

1. [Optional] Create a Python Virtual Environment - generally once per workspace

    ``` bash
    python -m venv <name of virtual environment>
    ```

2. [Optional] Activate Virtual Environment - each time new shell opened
    - Linux

      ```bash
      source <name of virtual environment>/bin/activate
      ```

    - Windows

      ``` bash
      <name of virtual environment>/Scripts/activate.bat
      ```

3. Install Pytools - generally once per virtual env or whenever pip-requirements.txt changes

    ``` bash
    pip install --upgrade -r pip-requirements.txt
    ```

4. Initialize & Update Submodules - only when submodules updated

    ``` bash
    stuart_setup -c Platforms/QemuQ35Pkg/PlatformBuild.py TOOL_CHAIN_TAG=<TOOL_CHAIN_TAG>
    ```

5. Initialize & Update Dependencies - only as needed when ext_deps change

    ``` bash
    stuart_update -c Platforms/QemuQ35Pkg/PlatformBuild.py TOOL_CHAIN_TAG=<TOOL_CHAIN_TAG>
    ```

6. Compile Firmware

    ``` bash
    stuart_build -c Platforms/QemuQ35Pkg/PlatformBuild.py TOOL_CHAIN_TAG=<TOOL_CHAIN_TAG>
    ```

    - use `stuart_build -c Platforms/QemuQ35Pkg/PlatformBuild.py-h` option to see additional
    options like `--clean`

7. Running Emulator
    - You can add `--FlashRom` to the end of your build command and the emulator will run after the
    build is complete.
    - or use the `--FlashOnly` feature to just run the emulator.

      ``` bash
      stuart_build -c Platforms/QemuQ35Pkg/PlatformBuild.py TOOL_CHAIN_TAG=<TOOL_CHAIN_TAG> --FlashOnly
      ```

### Notes

1. Configuring *ACTIVE_PLATFORM* and *TARGET_ARCH* in Conf/target.txt is **not** required. This
   environment is set by PlatformBuild.py.
2. QEMU must be on your path.  On Windows this is a manual process and not part of the QEMU installer.

**NOTE:** Logging the execution output will be in the normal stuart log as well as to your console.

### Custom Build Options

**MAKE_STARTUP_NSH=TRUE** will output a *startup.nsh* file to the location mapped as fs0. This is
used in CI in combination with the `--FlashOnly` feature to run QEMU to the UEFI shell and then execute
the contents of *startup.nsh*.

**QEMU_HEADLESS=TRUE** Since CI servers run headless QEMU must be told to run with no display otherwise
an error occurs. Locally you don't need to set this.

### Passing Build Defines

To pass build defines through _stuart_build_, prepend `BLD_*_`to the define name and pass it on the
command-line. _stuart_build_ currently requires values to be assigned, so add an`=1` suffix for bare defines.
For example, to enable the E1000 network support, instead of the traditional "-D E1000_ENABLE", the stuart_build
command-line would be:

`stuart_build -c Platforms/QemuQ35Pkg/PlatformBuild.py BLD_*_E1000_ENABLE=1`

## References

- [Installing and using Pytools](https://github.com/tianocore/edk2-pytool-extensions/blob/master/docs/using.md#installing)
- More on [python virtual environments](https://docs.python.org/3/library/venv.html)
