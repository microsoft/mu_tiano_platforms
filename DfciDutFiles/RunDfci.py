##
# This command make it easier to boot to a Windows device under test.
#
# See the ReadMe.md file in this directory on how to use the RunDfci script.
#
# Copyright (c) Microsoft Corporation
# SPDX-License-Identifier: BSD-2-Clause-Patent
##

if __name__ == "__main__":
    import os
    import argparse
    from pathlib import Path

    parser = argparse.ArgumentParser(description='Start Qemu with DFCI Options')

    # Add a usb drive with OS Install Files, or Dfci Setup files
    parse_group = parser.add_mutually_exclusive_group()
    parse_group.add_argument("-f", "--frontpage", dest="frontpage", action='store_true', help="Boot to FrontPage")
    parse_group.add_argument("-a", "--alt_boot", dest="alt_boot", action='store_true', help="Boot to Alternate source (USB/Network)")

    # Choose to boot to front page, or a usb/network device.
    parse_group2 = parser.add_mutually_exclusive_group()
    parse_group2.add_argument("-i", "--install", dest="install", action='store_true', help="Add drive with install files")
    parse_group2.add_argument("-d", "--dfcisetup", dest="dfcisetup", action='store_true', help="Add DfciSetup directory")

    options = parser.parse_args()

    dfci_directory = Path(__file__).parent.absolute()
    build_directory = dfci_directory.parent.absolute()

    platformbuild = os.path.join(build_directory, "Platforms", "QemuQ35Pkg", "PlatformBuild.py")

    dfci_var_store = os.path.join(dfci_directory, "DFCI_DUT_VARS.fd")

    args = " --FlashOnly"
    args += " DFCI_VAR_STORE=" + "\"" + dfci_var_store + "\""

    dfci_files = None
    install_files = None
    if options.install:
        install_files = os.path.join(dfci_directory, "OsInstallFiles.vhd")
        args += " INSTALL_FILES=" + "\"" + install_files + "\""

    if options.dfcisetup:
        dfci_files = os.path.join(dfci_directory, "DfciSetup")
        args += " DFCI_FILES=" + "\"" + dfci_files + "\""

    dfci_os_disk = os.path.join(dfci_directory, "Windows.vhd")

    if not os.path.exists(dfci_os_disk):
        raise Exception("The Windows.vhd file must exist")

    args += " PATH_TO_OS=" + "\"" + dfci_os_disk + "\""

    if options.frontpage:
        args += " BOOT_TO_FRONT_PAGE=TRUE"

    if options.alt_boot:
        args += " ALT_BOOT_ENABLE=TRUE"

    args += " BLD_*_QEMU_CORE_NUM=4"
    args += " BLD_*_SMM_ENABLED=TRUE"

    cmd = platformbuild + args
    os.system(cmd)
