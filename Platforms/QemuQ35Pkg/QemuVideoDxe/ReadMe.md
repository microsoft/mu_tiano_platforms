# Qemu Video Dxe

## About

This driver is derived from sample GOP driver QemuVideoDxe in OvmfPkg.
It replaces the standard GOP interfaces GUID with MsGopOverrideProtocolGuid from Project Mu to allow further
graphics control through Mu interfaces. It also removes support for BOCHS due to out of project scope.

## Copyright

Copyright (C) Microsoft Corporation.
SPDX-License-Identifier: BSD-2-Clause-Patent
