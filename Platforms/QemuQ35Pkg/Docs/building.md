# Building

Steps to setup your environment, compile, and run the QemuQ35Pkg.

This solution for building and running QemuQ35Pkg has only been validated with Windows 10
with VS2019 and Ubuntu 18.04 with GCC5 toolchain.

## Developer environment

This is a Project Mu platform and thus the default environment requirements can be found
here at the [Project Mu Prerequisites page.](https://microsoft.github.io/mu/CodeDevelopment/prerequisites/)

In addition if you want to run your locally compiled firmware you need

- [QEMU - Download, Install, and add to your path](https://www.qemu.org/download/)

This build uses edk2-pytools for functionality.  Documentation can be
found [here](https://github.com/tianocore/edk2-pytool-extensions/tree/master/docs).

### Additional Notes on Linux

This platform builds with edk2-pytools which supports both Windows and Linux environments.  Each
distro has its own configuration and setup but here are a few helpful hints to get started
to resolve dependencies.  If you can build Edk2 based firmware you should be pretty close.  

A newer version of *Mono* is currently required to support pytools based external dependencies (using nuget). More details are available here <https://github.com/tianocore/edk2-pytool-extensions/blob/master/docs/usability/using_extdep.md#a-note-on-nuget-on-linux>

Additionally on Ubuntu 18.04 this command has resolved many missing packages.

```bash
apt-get install gcc g++ make uuid-dev qemu
```

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

4. Initialize & Update Submodules - only when submodules are added/removed

    ``` bash
    stuart_setup -c Platforms/QemuQ35Pkg/PlatformBuild.py TOOL_CHAIN_TAG=<TOOL_CHAIN_TAG>
    ```

    - `TOOL_CHAIN_TAG` being the toolchain you want to build with, currently `VS2017`, `VS2019`, and `GCC5` are supported values

5. Initialize & Update Dependencies - only as needed when ext_deps change

    ``` bash
    stuart_update -c Platforms/QemuQ35Pkg/PlatformBuild.py TOOL_CHAIN_TAG=<TOOL_CHAIN_TAG>
    ```

6. Compile Firmware

    ``` bash
    stuart_build -c Platforms/QemuQ35Pkg/PlatformBuild.py TOOL_CHAIN_TAG=<TOOL_CHAIN_TAG>
    ```

    - use `stuart_build -c Platforms/QemuQ35Pkg/PlatformBuild.py -h` option to see additional
    options like `--clean`

7. Running Emulator
    - You can add `--FlashRom` to the end of your build command and the emulator will run after the
    build is complete.
    - or use the `--FlashOnly` feature to just run the emulator.

      ``` bash
      stuart_build -c Platforms/QemuQ35Pkg/PlatformBuild.py TOOL_CHAIN_TAG=<TOOL_CHAIN_TAG> --FlashOnly
      ```

!!! info "Alternative invocation options"
    - All the commands specified here can use a shortcut, which is to invoke the Build file directly. For example:

    ``` bash
    py Platforms/QemuQ35Pkg/PlatformBuild.py TOOL_CHAIN_TAG=<TOOL_CHAIN_TAG>  --FlashOnly
    ```

    - Setup and update can be done by passing it in

      ``` bash
      py Platforms/QemuQ35Pkg/PlatformBuild.py TOOL_CHAIN_TAG=<TOOL_CHAIN_TAG>  --setup
      ```

      ``` bash
      py Platforms/QemuQ35Pkg/PlatformBuild.py TOOL_CHAIN_TAG=<TOOL_CHAIN_TAG>  --update
      ```

    - Under the hood, it just does the invocation of Stuart for you.

!!! note "Notes"
    1. QEMU **must** be on your path.  On Windows this is a manual process and not part of the QEMU installer.
    2. QEMU build output will be in `Build/BUILDLOG_QemuQ35Pkg.txt`.
    3. In cases where the `--flashonly` or `--flashrom` parameters were used to invoke the runner and
       a startup.nsh was requested, the output log will changed to `Build/BUILDLOG_QemuQ35Pkg_With_Run.txt`
    3. Logging to your local console window can be controlled and is often not as detailed as the log files.

### Passing Build Defines

To pass build defines through *stuart_build*, prepend `BLD_*_` to the define name and pass it on the
command-line. *stuart_build* currently requires values to be assigned, so add a `=1` suffix for bare defines.
For example, to enable the E1000 network support, instead of the traditional "-D E1000_ENABLE", the stuart_build
command-line would be:

`stuart_build -c Platforms/QemuQ35Pkg/PlatformBuild.py BLD_*_E1000_ENABLE=1`

## Advanced options for automation and testing

The QemuQ35Pkg platform uses the QemuRunner plugin for executing QEMU with the
locally compiled firmware.  This runner has numerous configuration options which allow
for automating unit test execution or other efi application execution.  See the plugin details
for a complete list of capabilities and configurations.  [Platforms/QemuQ35Pkg/Plugins/QemuRunner](../Platforms/QemuQ35Pkg/Plugins/QemuRunner/ReadMe.md)

## References

- [Installing and using Pytools](https://github.com/tianocore/edk2-pytool-extensions/blob/master/docs/using.md#installing)
- More on [python virtual environments](https://docs.python.org/3/library/venv.html)
- [Qemu Documentation](https://www.qemu.org/docs/master/)
