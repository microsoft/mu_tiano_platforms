# Building

Steps to setup your environment, compile, and run QemuQ35Pkg and QemuSbsaPkg.

## Developer environment

This is a Project Mu platform and thus the default environment requirements can be found
here at the [Project Mu Prerequisites page.](https://microsoft.github.io/mu/CodeDevelopment/prerequisites/)

QEMU is used to run the locally compiled firmware on a virtual platform. If you are on windows,
no action is needed, we provide an [external dependency](https://www.tianocore.org/edk2-pytool-extensions/features/extdep/) that includes the necessary QEMU binaries.
If you are on Linux, [install it](https://www.qemu.org/download/#linux).

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
    stuart_setup -c Platforms/<Package>/PlatformBuild.py TOOL_CHAIN_TAG=<TOOL_CHAIN_TAG>
    ```

    - `TOOL_CHAIN_TAG` being the toolchain you want to build with, currently `VS2019`, `VS2022`, and `GCC5` are
      supported values. Q35 can be built with `GCC5`, `VS2019`, and `VS2022` toolchains. SBSA can be built with
      `GCC5`.
    - NOTE: Building with GCC5 targeting AARCH64 requires the setup to export `GCC5_AARCH64_PREFIX`.
      `GCC5_AARCH64_PREFIX` should contain the path and the prefix to the gcc/objcopy tools, and needs to
      match the tool chain that is being used by the platform. The below example is for the gcc-aarch64-linux-gnu
      tool chain, but other tool chains can be used.
      As an example, `export GCC5_AARCH64_PREFIX=/usr/bin/aarch64-linux-gnu-`

5. Initialize & Update Dependencies - only as needed when ext_deps change

    ``` bash
    stuart_update -c Platforms/<Package>/PlatformBuild.py TOOL_CHAIN_TAG=<TOOL_CHAIN_TAG>
    ```

6. Compile Firmware

    ``` bash
    stuart_build -c Platforms/<Package>/PlatformBuild.py TOOL_CHAIN_TAG=<TOOL_CHAIN_TAG>
    ```

    - use `stuart_build -c Platforms/<Package>/PlatformBuild.py -h` option to see additional
    options like `--clean`

7. Running Emulator
    - You can add `--FlashRom` to the end of your build command and the emulator will run after the
    build is complete.
    - or use the `--FlashOnly` feature to just run the emulator.

      ``` bash
      stuart_build -c Platforms/<Package>/PlatformBuild.py TOOL_CHAIN_TAG=<TOOL_CHAIN_TAG> --FlashOnly
      ```

8. Alternative Options
    - All the commands specified here can use a shortcut, which is to invoke the Build file directly. For example:

      ``` bash
      py Platforms/<Package>/PlatformBuild.py TOOL_CHAIN_TAG=<TOOL_CHAIN_TAG>  --FlashOnly
      ```

    - Setup and update can be done by passing it in

      ``` bash
      py Platforms/<Package>/PlatformBuild.py TOOL_CHAIN_TAG=<TOOL_CHAIN_TAG>  --setup
      ```

      ``` bash
      py Platforms/<Package>/PlatformBuild.py TOOL_CHAIN_TAG=<TOOL_CHAIN_TAG>  --update
      ```

    - Under the hood, it just does the invocation of Stuart for you.

### Notes

1. QEMU is provided on windows via an external dependency located at QemuPkg/Binaries; Qemu must be manually downloaded
   on linux.
2. QEMU for linux requires at least **version 9.0.2** when booting an operating system; if you are only booting to
   shell, matching the version to the windows external dependency is acceptable.
3. If you want to override the external dependency on windows, or the installed version on linux, you can use
   `QEMU_PATH = <path>` on the command line.

**NOTE:** Logging the execution output will be in the normal stuart log as well as to your console (if you have the
correct logging level set, by default it doesn't output to console).

### Custom Build Options

**SHUTDOWN_AFTER_RUN=TRUE** will output a *startup.nsh* file to the location mapped as fs0 with `reset -s` as
the final line. This is used in CI in combination with the `--FlashOnly` feature to run QEMU to the UEFI shell
and then execute the contents of *startup.nsh*.

**QEMU_PATH** Can specify the path to a specific QEMU binary to use.

**QEMU_HEADLESS=TRUE** Since CI servers run headless QEMU must be told to run with no display otherwise
an error occurs. Locally you don't need to set this.

**GDB_SERVER=\<TCP Port\>** Enables the GDB port in the QEMU instance at the provided TCP port.

**SERIAL_PORT=\<Serial Port\>** Enables the specified serial port to be used as console.

**ENABLE_NETWORK=TRUE** will enable networking (currently supported on the QEMU Q35 platform). If `DFCI_VAR_STORE` is
set, networking will also be enabled with TCP ports 8270 and 8271 forwarded for the robot framework.

### Passing Build Defines

To pass build defines through *stuart_build*, prepend `BLD_*_` to the define name and pass it on the
command-line. *stuart_build* currently requires values to be assigned, so add a `=1` suffix for bare defines.
For example, to enable the E1000 network support, instead of the traditional "-D E1000_ENABLE", the stuart_build
command-line would be:

`stuart_build -c Platforms/<Package>/PlatformBuild.py BLD_*_E1000_ENABLE=1`

## References

- [Installing and using Pytools](https://www.tianocore.org/edk2-pytool-extensions/using/install/)
- More on [python virtual environments](https://docs.python.org/3/library/venv.html)
