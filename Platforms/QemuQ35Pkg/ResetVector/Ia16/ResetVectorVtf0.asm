;------------------------------------------------------------------------------
; @file
; First code executed by processor after resetting.
; Derived from UefiCpuPkg/ResetVector/Vtf0/Ia16/ResetVectorVtf0.asm
;
; Copyright (c) 2008 - 2014, Intel Corporation. All rights reserved.<BR>
; SPDX-License-Identifier: BSD-2-Clause-Patent
;
;------------------------------------------------------------------------------

BITS    16

ALIGN   16

;
; Pad the image size to 4k when page tables are in VTF0
;
; If the VTF0 image has page tables built in, then we need to make
; sure the end of VTF0 is 4k above where the page tables end.
;
; This is required so the page tables will be 4k aligned when VTF0 is
; located just below 0x100000000 (4GB) in the firmware device.
;
%ifdef ALIGN_TOP_TO_4K_FOR_PAGING
    TIMES (0x1000 - ($ - EndOfPageTables) - 0x20) DB 0
%endif

ALIGN   16
;
; 0xffffffe0
;
   DD      0, 0, 0

;
; The VTF signature (0xffffffec)
;
; VTF-0 means that the VTF (Volume Top File) code does not require
; any fixups.
;
vtfSignature:
    DB      'V', 'T', 'F', 0

ALIGN   16

resetVector:
;
; Reset Vector
;
; This is where the processor will begin execution
;
; In IA32 we follow the standard reset vector flow. While in X64, Td guest
; may be supported. Td guest requires the startup mode to be 32-bit
; protected mode but the legacy VM startup mode is 16-bit real mode.
; To make NASM generate such shared entry code that behaves correctly in
; both 16-bit and 32-bit mode, more BITS directives are added.
;
%ifdef ARCH_IA32
    nop
    nop
    jmp     EarlyBspInitReal16

%else

    mov     eax, cr0
    test    al, 1
    jz      .Real
BITS 32
    jmp     Main32
BITS 16
.Real:
    jmp     EarlyBspInitReal16

%endif

ALIGN   16

fourGigabytes:

