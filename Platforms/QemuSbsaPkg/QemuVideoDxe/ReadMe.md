# Qemu Video Dxe

## About

This driver is derived from the sample GOP driver QemuVideoDxe in QemuQ35Pkg.
It replaces the standard GOP interfaces GUID with MsGopOverrideProtocolGuid from Project Mu to allow further
graphics control through Mu interfaces. It removes support for Cirrus in favor of BOCHS used by SBSA.

## Copyright

Copyright (C) Microsoft Corporation.
SPDX-License-Identifier: BSD-2-Clause-Patent
