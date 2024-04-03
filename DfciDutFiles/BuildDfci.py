##
# This command make it easier to build with DFCI enabled and match the parameters with RunDfci.py.
#
# See the ReadMe.md file in this directory on how to use the BuildDfci script.
#
# Copyright (c) Microsoft Corporation
# SPDX-License-Identifier: BSD-2-Clause-Patent
##

if __name__ == "__main__":
    import os
    from pathlib import Path

    dfci_directory = Path(__file__).parent.absolute()
    build_directory = dfci_directory.parent.absolute()
    platformbuild = os.path.join(build_directory, "Platforms", "QemuQ35Pkg", "PlatformBuild.py")

    args = " BLD_*_GUI_FRONT_PAGE=TRUE"
    args += " BLD_*_NETWORK_ALLOW_HTTP_CONNECTIONS=TRUE"
    args += " BLD_*_QEMU_CORE_NUM=4"
    args += " BLD_*_SMM_ENABLED=TRUE"
    args += " --clean"

    cmd = platformbuild + args
    os.system(cmd)
