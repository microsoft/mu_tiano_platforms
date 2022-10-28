/*++

Copyright (c) Microsoft Corporation

Module Name:

    modules.cpp

Abstract:

    This file contains debug commands for enumerating UEFI modules and their
    symbols.

--*/

#include "uefiext.h"

HRESULT CALLBACK
findmodules(PDEBUG_CLIENT4 Client, PCSTR args)
{
    ULONG64 HeaderAddress;
    UINT32 TableSize;
    ULONG64 Table;
    ULONG64 Entry;
    ULONG64 NormalImage;
    ULONG64 ImageProtocol;
    UINT64 ImageBase;
    ULONG Index;
    CHAR Command[512];
    INIT_API();

    UNREFERENCED_PARAMETER(args);

    //
    // TODO: Add support for PEI & MM
    //

    if (gUefiEnv != DXE) {
        dprintf("Only supported for DXE!\n");
        return ERROR_NOT_SUPPORTED;
    }

    HeaderAddress = GetExpression("&mDebugInfoTableHeader");
    if (HeaderAddress == NULL) {
        dprintf("Failed to find mDebugInfoTableHeader!\n");
        return ERROR_NOT_FOUND;
    }

    GetFieldValue(HeaderAddress, "EFI_DEBUG_IMAGE_INFO_TABLE_HEADER", "TableSize", TableSize);
    GetFieldValue(HeaderAddress, "EFI_DEBUG_IMAGE_INFO_TABLE_HEADER", "EfiDebugImageInfoTable", Table);
    if ((Table == NULL) || (TableSize == 0)) {
        dprintf("Debug table is empty!\n");
        return ERROR_NOT_FOUND;
    }

    // Skip the 0-index to avoid reloading DxeCore. There is probably a better way to do this.
    for (Index = 1; Index < TableSize; Index++) {
        Entry = Table + (Index * GetTypeSize("EFI_DEBUG_IMAGE_INFO"));
        GetFieldValue(Entry, "EFI_DEBUG_IMAGE_INFO", "NormalImage", NormalImage);
        if (NormalImage == NULL) {
            dprintf("Skipping missing normal info!\n");
            continue;
        }

        GetFieldValue(NormalImage, "EFI_DEBUG_IMAGE_INFO_NORMAL", "LoadedImageProtocolInstance", ImageProtocol);
        if (ImageProtocol == NULL) {
            dprintf("Skipping missing loaded image protocol!\n");
            continue;
        }

        GetFieldValue(ImageProtocol, "EFI_LOADED_IMAGE_PROTOCOL", "ImageBase", ImageBase);
        sprintf_s(Command, sizeof(Command), ".imgscan /l /r %I64x (%I64x + 0xFFF)", ImageBase, ImageBase);
        g_ExtControl->Execute(DEBUG_OUTCTL_ALL_CLIENTS,
                              Command,
                              DEBUG_EXECUTE_DEFAULT);
    }

    EXIT_API();
    return S_OK;
}
