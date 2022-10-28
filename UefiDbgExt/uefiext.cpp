/*++

Copyright (c) Microsoft Corporation

Module Name:

    uefiext.cpp

Abstract:

    This file contains core UEFI debug commands.

--*/

#include "uefiext.h"

UEFI_ENV gUefiEnv = DXE;

HRESULT
NotifyOnTargetAccessible(PDEBUG_CONTROL Control)
{

    //
    // Attempt to determine what environment the debugger is in.
    //

    return S_OK;
}

HRESULT CALLBACK
setenv(PDEBUG_CLIENT4 Client, PCSTR args)
{
    INIT_API();

    if (_stricmp(args, "PEI") == 0) {
        gUefiEnv = PEI;
    } else if (_stricmp(args, "DXE") == 0) {
        gUefiEnv = DXE;
    } else if (_stricmp(args, "MM") == 0) {
        gUefiEnv = MM;
    } else {
        dprintf("Unknown environment type! Supported types: PEI, DXE, MM");
    }

    EXIT_API();
    return S_OK;
}

HRESULT CALLBACK
help(PDEBUG_CLIENT4 Client, PCSTR args)
{
    INIT_API();

    UNREFERENCED_PARAMETER(args);

    dprintf("Help for uefiext.dll\n"
            "  help                - Shows this help\n"
            "  memorymap           - Prints the current memory map\n"
            "  findmodules         - Find and loads symbols for all modules in the debug list\n"
            "  setenv              - Set the extensions environment mode\n"
            "  hobs                - Enumerates the hand off blocks\n"
            );

    EXIT_API();

    return S_OK;
}
