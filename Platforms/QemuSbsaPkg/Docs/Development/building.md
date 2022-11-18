# Building

Steps to setup your environment, compile, and run the QemuSbsaPkg.

This solution for building and running QemuSbsaPkg has only been validated with Windows 10
with VS2019 and Ubuntu 18.04 with GCC5 toolchain.

## Developer environment

This is a Project Mu platform and thus the default environment requirements can be found
here at the [Project Mu Prerequisites page.](https://microsoft.github.io/mu/CodeDevelopment/prerequisites/)

In addition if you want to run your locally compiled firmware you need

- [QEMU - Download, Install, and add to your path](https://www.qemu.org/download/)

This build uses edk2-pytools for functionality.  Documentation can be
found [here](https://github.com/tianocore/edk2-pytool-extensions/tree/master/docs).
On most Linux distros this requires an extra step for mono and nuget support.
<https://github.com/tianocore/edk2-pytool-extensions/blob/master/docs/usability/using_extdep.md#a-note-on-nuget-on-linux>

## Building with Pytools

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
    stuart_setup -c Platforms/QemuSbsaPkg/PlatformBuild.py TOOL_CHAIN_TAG=<TOOL_CHAIN_TAG>
    ```

    - `TOOL_CHAIN_TAG` being the toolchain you want to build with, currently `VS2017`, `VS2019`, and `GCC5` are
      supported values

5. Initialize & Update Dependencies - only as needed when ext_deps change

    ``` bash
    stuart_update -c Platforms/QemuSbsaPkg/PlatformBuild.py TOOL_CHAIN_TAG=<TOOL_CHAIN_TAG>
    ```

6. Compile Firmware

    ``` bash
    stuart_build -c Platforms/QemuSbsaPkg/PlatformBuild.py TOOL_CHAIN_TAG=<TOOL_CHAIN_TAG>
    ```

    - use `stuart_build -c Platforms/QemuSbsaPkg/PlatformBuild.py -h` option to see additional
    options like `--clean`

7. Running Emulator
    - You can add `--FlashRom` to the end of your build command and the emulator will run after the
    build is complete.
    - or use the `--FlashOnly` feature to just run the emulator.

      ``` bash
      stuart_build -c Platforms/QemuSbsaPkg/PlatformBuild.py TOOL_CHAIN_TAG=<TOOL_CHAIN_TAG> --FlashOnly
      ```

8. Alternative Options
    - All the commands specified here can use a shortcut, which is to invoke the Build file directly. For example:

      ``` bash
      py Platforms/QemuSbsaPkg/PlatformBuild.py TOOL_CHAIN_TAG=<TOOL_CHAIN_TAG>  --FlashOnly
      ```

    - Setup and update can be done by passing it in

      ``` bash
      py Platforms/QemuSbsaPkg/PlatformBuild.py TOOL_CHAIN_TAG=<TOOL_CHAIN_TAG>  --setup
      ```

      ``` bash
      py Platforms/QemuSbsaPkg/PlatformBuild.py TOOL_CHAIN_TAG=<TOOL_CHAIN_TAG>  --update
      ```

    - Under the hood, it just does the invocation of Stuart for you.

### Notes

1. QEMU must be on your path.  On Windows this is a manual process and not part of the QEMU installer.
2. QEMU output will be in Build/BUILDLOG_QemuSbsaPkg.txt as well as Build/QemuSbsaPkg/QEMULOG_QemuSbsaPkg.txt

**NOTE:** Logging the execution output will be in the normal stuart log as well as to your console (if you have the
correct logging level set, by default it doesn't output to console).

### Custom Build Options

**SHUTDOWN_AFTER_RUN=TRUE** will output a *startup.nsh* file to the location mapped as fs0 with `reset -s` as
the final line. This is used in CI in combination with the `--FlashOnly` feature to run QEMU to the UEFI shell
and then execute the contents of *startup.nsh*.

**QEMU_HEADLESS=TRUE** Since CI servers run headless QEMU must be told to run with no display otherwise
an error occurs. Locally you don't need to set this.

**GDB_SERVER=\<TCP Port\>** Enables the GDB port in the QEMU instance at the provided TCP port.

### Passing Build Defines

To pass build defines through *stuart_build*, prepend `BLD_*_` to the define name and pass it on the
command-line. *stuart_build* currently requires values to be assigned, so add a `=1` suffix for bare defines.
For example, to enable the E1000 network support, instead of the traditional "-D E1000_ENABLE", the stuart_build
command-line would be:

`stuart_build -c Platforms/QemuSbsaPkg/PlatformBuild.py BLD_*_E1000_ENABLE=1`

## References

- [Installing and using Pytools](https://github.com/tianocore/edk2-pytool-extensions/blob/master/docs/using.md#installing)
- More on [python virtual environments](https://docs.python.org/3/library/venv.html)
