
# Debugger QEMU Platforms

This document described different ways to debug on the QEMU platforms.

## Debugging using UEFI debugger

Both Q35 and SBSA are setup to debug debugged using the MU debugger package. for
more details on using this debugger, see the [FeatureDebuggerPkg Readme](https://github.com/microsoft/mu_feature_debugger/blob/main/DebuggerFeaturePkg/Readme.md).

By default the debugger is disabled, to enable you must both enable the debugger
build flag to enable the source debug device state flag and specify a serial port
TCP port.

```
stuart_build -c Platforms\QemuQ35Pkg\PlatformBuild.py BLD_*_DEBUGGER_ENABLED=TRUE SERIAL_PORT=5555 --flashrom
```

On Q35 this allows for debugging over a different port then the usual debug output
because Q35 has a seperate serial port available to it. On SBSA the serial port
will be shared with the logging output.

Currently this will only enable the DXE debugger. The MM debugger must be manually
enabled using the PcdForceEnableDebugger if the DebugAgent has been configured.
By default, the DXE debugger will stall for 30 seconds on the initial breakpoint
before attempting to continue execution.

## Debugging using QEMU GDB Server

QEMU has the ability to expose a GDB port for debugging. This can be leveraged
several ways. To start enable the GDB server, add the following parameter when
launched QEMU through the QEMU runner.

    GDB_SERVER=<port number>

Example ports can be: 1234, 5000, 5001, etc.

## Windbg Integration

Windbg supports source debugging QEMU through an EXDI interface with the GDB
port. Details can be found on the [github readme](https://github.com/microsoft/WinDbg-Samples/blob/master/Exdi/exdigdbsrv/doc/ExdiGdbSrv_readme.md).
This supplies a EXDI server binary, a configuration file, and a script to
start Windbg bound to the EXDI interface.

Once Windbg is connected, following the instruction in the readme, the symbols
and source can be loaded. This is most easily done using the
[UEFI debugger extension](../../../UefiDbgExt/readme.md). This can also be done
manually by scanning memory for images using the image scan command. For example:

    kd> r rip
    rip=000000007d1fc64b
    kd> .imgscan /l /r 0x07d000000 0x07e000000

This will scan for image headers between the specified addresses and load their
symbols. More information on this command can be found in the Windbg help
window.

## Debugging Using GDB in VS Code

For GCC builds, GDB can be used to debug instead. The symbols can be loaded by
running `source MU_BASECORE/BaseTools/Scripts/efi_gdb.py` from within GDB while
stopped. To connect GDB to the built in VS Code debugger, you can use the following
launch configuration to connect to a running instance of QEMU.

```json
{
    "name": "Connect to GDB Server (X64)",
    "type": "cppdbg",
    "request": "launch",
    "program": "${workspaceRoot}/Build/QemuQ35Pkg/DEBUG_GCC5/X64/DxeCore.debug",
    "miDebuggerServerAddress": "localhost:1234",
    "cwd": "${workspaceRoot}",
    "environment": [],
    "MIMode": "gdb",
    "miDebuggerPath": "gdb",
    "stopAtConnect": true
},
{
    "name": "Connect to GDB Server (AARCH64)",
    "type": "cppdbg",
    "request": "launch",
    "program": "${workspaceRoot}/Build/QemuSbsaPkg/DEBUG_GCC5/AARCH64/DxeCore.debug",
    "miDebuggerServerAddress": "localhost:1234",
    "cwd": "${workspaceRoot}",
    "environment": [],
    "MIMode": "gdb",
    "miDebuggerPath": "gdb-multiarch",
    "stopAtConnect": true
},
```

Note that the `program` value must be a legitimate binary, but does not seem to
have any affect on the debugging. Additionally, `gdb` may need to be used instead
of `gdb` if debugging x64 from an x64 machine.

Once attached, you can stop and run `-exec source MU_BASECORE/BaseTools/Scripts/efi_gdb.py`
from the DEBUG CONSOLE tab to load the EFI symbols and commands. You may need to
single step after this for symbols to take affect.

## Debugging Windows on QEMU

### Boot to OS

In order to kernel debug the OS, once need to boot to OS first.

1. Download original OS image in the format of VHDX or QCOW2. If the image has never
been booted before, one can boot this image using Hyper-V once to get through the
OOBE process, if any. Related reads:

    - [Enable Hyper-V on Windows 10](https://docs.microsoft.com/en-us/virtualization/hyper-v-on-windows/quick-start/enable-hyper-v)
    - [Create a Virtual Machine with Hyper-V](https://docs.microsoft.com/en-us/virtualization/hyper-v-on-windows/quick-start/quick-create-virtual-machine#windows-10-fall-creators-update-windows-10-version-1709)

1. Although QEMU supports both VHDX and QCOW2 images. For the sake of fail proof,
it is recommended to use QCOW2 format image, which can be converted from VHDX
image with:

    `qemu-img convert -f vhdx -p -c -O qcow2 foo.vhdx foo.qcow2`

1. With the above QCOW2 image, set up the `PATH_TO_OS` parameter when launching
QemuRunner.py:

    `PATH_TO_OS=<absolute path to your OS image> SERIAL_PORT=<port number>`

    - More details on the `<port number>` in the [corresponding section](#configurations-on-host-that-runs-qemu)

### Debug Windows with WinDbg on QEMU

Unlike EXDI debugger introduced above, this method will enlighten the target OS
and debug the Windows more "traditionally", where Windows can communicate to the
attached debugger with proper transports.

#### Configurations on Target QEMU

After booting to the desktop or command prompt of target Windows by following
[above steps](#boot-to-os), issue below commands:

    bcdedit /dbgsettings serial debugport:1 baudrate:115200
    bcdedit /set {default} debug on
    # One can potentially enable boot debugger as well
    bcdedit /set {default} bootdebug on

#### Configurations on Host that runs QEMU

Once the QEMU boots to the point where the target is accepting debugger commands,
launch the WinDbg using:

`windbg.exe -k com:ipport=<port number>,port=127.0.0.1 -v`

where the `<port number>` is the number you set when [launching QEMU](#boot-to-os)

### Serial Console for UEFI

Launch terminal application such as Putty or Tera Term to connect to `<port number>`
you set when [launching QEMU](#boot-to-os) at `127.0.0.1` through raw TCP/IP
protocol.

Note:

- One needs to release this console in order for the KD to attach.
- Some terminal software would enable "local line editing" for raw connection,
this needs to be turned off to prevent garbage keystrokes.
