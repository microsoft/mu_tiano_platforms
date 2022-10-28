
# UEFI Debugger Extension

This folder contains the source for the UEFI debugger extension. This provides
functionality within windbg for debugging the UEFI environment. Using the UEFI
extension requires that windbgx has access to the symbol files for the target
UEFI code.

## Compiling

Windbg debugger extensions need to be compiled with the Microsoft build tools.
The easiest way to do this is to use the Developer Command Prompt that comes
with the Visual Studio tools installation. In the command prompt, navigate to
the folder and run "msbuild".

    msbuild -property:Configuration=Release -property:Platform=x64

The project can also be loaded and built in Visual Studio using the solution
file.

## Installing the Extension

Debugger extensions can be loaded into windbg several ways. First, by
manually loading once already in windbg. This can be done with the .load
command. Though this will not persist across windbg sessions.

    .load <path to uefiext.dll>

The second is to place the DLL in the windbg application folder, or another
place in windbg's extpath which can be enumerating using the .extpath command.
This will make the extension available to all future windbg sessions.

    e.g. C:\Users\<user>\AppData\Local\dbg\UI

For more information about loading debugger extensions see the
[Microsoft documentation page](https://docs.microsoft.com/en-us/windows-hardware/drivers/debugger/loading-debugger-extension-dlls).

## Using the Extension

Once the extension is loaded into windbg, you can use it by running any of its
commands. To see its commands, use the help command to get started.

    !uefiext.help

## Creating new commands

New commands can be exported by added them to the exports in uefiext.def. New
commands should also be added to the help command in uefiext.cpp. For reference
on how to write debugger extension code, see the [Microsoft API Docs](https://docs.microsoft.com/en-us/windows-hardware/drivers/debugger/debugger-engine-and-extension-apis).
