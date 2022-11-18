
# Debugging QEMU

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
and source can be loaded by scanning memory for images using the image scan
command. For example:

    kd> r pc
    pc=000000007d1fc64b
    kd> .imgscan /l /r 0x07d000000 0x07e000000

This will scan for image headers between the specified addresses and load their
symbols. More information on this command can be found in the Windbg help
window.
