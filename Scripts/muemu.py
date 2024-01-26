#
#  Script for running QEMU with the appropriate options for the given SKU/ARCH.
#
#  Copyright (c) Microsoft Corporation
#  SPDX-License-Identifier: BSD-2-Clause-Patent
#

import zipfile
import urllib.request
import sys
import subprocess
import shutil
import requests
import os
import argparse
from typing import List

#
# Constants
#

DEFAULT_VERSION = "4.7.3"

#
# Setup and parse arguments.
#

parser = argparse.ArgumentParser()

# HINT: Run with '--help all' to get complete help.
parser.add_argument("-u", "--update", action="store_true",
                    help="Updates the firmware binaries.")
parser.add_argument("--firmwaredir", default="./firmware",
                    help="Directory to download and use firmware binaries.")
parser.add_argument("-a", "--arch", default="x64",
                    choices=["x64", "arm64"], help="The guest architecture for the VM.")
parser.add_argument("-d", "--disk",
                    help="Path to the disk file.")
parser.add_argument("-c", "--cores", default=2, type=int,
                    help="The number of cores for the VM. This may be overridden based on the configuration.")
parser.add_argument("-m", "--memory", default="4096",
                    help="The memory size to use in Mb.")
parser.add_argument("--tpm", action="store_true",
                    help="Linux only. Enables the emulated TPM using swtpm.")
parser.add_argument("--vnc",
                    help="Provides the VNC port to use based on 5900. E.g. ':1' for localhost:5901")
parser.add_argument("--accel", default="tcg",
                    choices=["tcg", "kvm", "whpx"], help="Acceleration back-end to use in QEMU.")
parser.add_argument("--version", default=DEFAULT_VERSION,
                    help="The Project MU firmware version to use.")
parser.add_argument("--qemudir", default="",
                    help="Path to a custom QEMU install directory.")
parser.add_argument("--gdbport", type=int,
                    help="Enables the GDB server on the specified port. E.g. 1234")
parser.add_argument("--debugfw", action="store_true",
                    help="Enables update to use the DEBUG firmware binaries.")
parser.add_argument("--verbose", action="store_true",
                    help="Enabled verbose script prints.")
parser.add_argument("--force", action="store_true",
                    help="Disables automatic correction of VM configurations.")
parser.add_argument("--timeout", type=int, default=None,
                    help="The number of seconds to wait before killing the QEMU process.")
parser.add_argument("--qemuargs", type=str, default=None,
                    help="Additional arguments to provide to QEMU, using the format \" -foo bar\". "
                    "The preceding space is required for parsing reasons.")

args = parser.parse_args()

#
# Script routines.
#

def main():
    # Run special operations if requested.
    if args.update:
        update_firmware()
        return

    # Build the platform specific arguments.
    qemu_args = []
    if args.arch == "x64":
        build_args_x64(qemu_args)
    elif args.arch == "arm64":
        build_args_arm64(qemu_args)
    else:
        raise ValueError(f"Invalid architecture '{args.arch}'!")

    # General device config
    qemu_args += ["-name", f"MU-{args.arch}"]
    qemu_args += ["-m", f"{args.memory}"]
    qemu_args += ["-smp", f"{args.cores}"]

    # SMBIOS
    qemu_args += ["-smbios", "type=0,vendor=Palindrome,uefi=on"]
    qemu_args += ["-smbios",
                  "type=1,manufacturer=Palindrome,product=MuQemu,serial=42-42-42-42"]

    # Storage
    if args.disk != None:
        qemu_args += ["-hda", f"{args.disk}"]

    # User input devices
    qemu_args += ["-device", "qemu-xhci,id=usb"]
    qemu_args += ["-device", "usb-mouse,id=input0,bus=usb.0,port=1"]
    qemu_args += ["-device", "usb-kbd,id=input1,bus=usb.0,port=2"]
    # qemu_args += ["-device", "usb-tablet"]

    # Network
    qemu_args += ["-nic", "model=e1000"]

    # TPM
    if args.tpm:
        qemu_args += ["-chardev",
                      f"socket,id=chrtpm,path={args.firmwaredir}/tpm/swtpm-sock"]
        qemu_args += ["-tpmdev", "emulator,id=tpm0,chardev=chrtpm"]
        qemu_args += ["-device", "tpm-tis,tpmdev=tpm0"]

    # Display
    if args.vnc != None:
        qemu_args += ["-display", f"vnc={args.vnc}"]

    # Debug & Serial ports
    if args.gdbport != None:
        qemu_args += ["-gdb", f"tcp::{args.gdbport}"]

    # User provided arguments
    if args.qemuargs != None:
        print(f"qemu args {args.qemuargs}")
        qemu_args.extend(args.qemuargs.split())

    # Launch QEMU
    run_qemu(qemu_args)


def build_args_x64(qemu_args: List[str]):
    smm_value = "off" if args.accel == "whpx" else "on"
    qemu_args += [f"{args.qemudir}qemu-system-x86_64"]
    qemu_args += ["-cpu", "qemu64,+rdrand,umip,+smep,+popcnt"]
    qemu_args += ["-global", "ICH9-LPC.disable_s3=1"]
    qemu_args += ["-machine", f"q35,smm={smm_value},accel={args.accel}"]
    qemu_args += ["-debugcon", "stdio"]  # file:uefi-x64.log
    qemu_args += ["-global", "isa-debugcon.iobase=0x402"]
    qemu_args += ["-vga", "cirrus"]

    # Flash storage
    if smm_value == "on":
        code_fd = f"{args.firmwaredir}/x64/QemuQ35/VisualStudio-x64/QEMUQ35_CODE.fd"
        data_fd = f"{args.firmwaredir}/x64/QemuQ35/VisualStudio-x64/QEMUQ35_VARS.fd"
        qemu_args += ["-global",
                      "driver=cfi.pflash01,property=secure,value=on"]
    else:
        print("Switching to no-SMM firmware for WHPX.")
        code_fd = f"{args.firmwaredir}/x64/QemuQ35.NoSmm/VisualStudio-NoSmm-x64/QEMUQ35_CODE.fd"
        data_fd = f"{args.firmwaredir}/x64/QemuQ35.NoSmm/VisualStudio-NoSmm-x64/QEMUQ35_VARS.fd"

    qemu_args += ["-drive",
                  f"if=pflash,format=raw,unit=0,file={code_fd},readonly=on"]
    qemu_args += ["-drive", f"if=pflash,format=raw,unit=1,file={data_fd}"]

    if args.cores > 4 and not args.force:
        print("Only 4 core currently supported for ARM64, setting cores to 4.")
        args.cores = 4


def build_args_arm64(qemu_args: List[str]):
    qemu_args += [f"{args.qemudir}qemu-system-aarch64"]
    qemu_args += ["-machine", f"sbsa-ref,accel={args.accel}"]
    qemu_args += ["-cpu", "max"]
    qemu_args += ["-serial", "stdio"]  # file:uefi-arm64.log

    # Flash storage
    sec_fd = f"{args.firmwaredir}/aarch64/QemuSbsa/GCC-AARCH64/SECURE_FLASH0.fd"
    efi_fd = f"{args.firmwaredir}/aarch64/QemuSbsa/GCC-AARCH64/QEMU_EFI.fd"
    qemu_args += ["-global", "driver=cfi.pflash01,property=secure,value=on"]
    qemu_args += ["-drive", f"if=pflash,format=raw,unit=0,file={sec_fd}"]
    qemu_args += ["-drive",
                  f"if=pflash,format=raw,unit=1,file={efi_fd},readonly=on"]

    if args.cores != 4 and not args.force:
        print("Arm64 image must use 4 cores! Setting to 4.")
        args.cores = 4


def run_qemu(qemu_args: List[str]):
    if args.tpm:
        os.makedirs(f"{args.firmwaredir}/tpm", exist_ok=True)
        swtpm_args = ["swtpm",
                      "socket",
                      "--tpmstate", f"dir={args.firmwaredir}/tpm",
                      "--ctrl", f"type=unixio,path={args.firmwaredir}/tpm/swtpm-sock",
                      "--tpm2",
                      "--log", f"file={args.firmwaredir}/tpm/tpm.log,level=20"]

        swtpm_proc = subprocess.Popen(swtpm_args)

    if args.verbose:
        print(qemu_args)
        subprocess.run([qemu_args[0], "--version"])
    try:
        subprocess.run(qemu_args, timeout=args.timeout)
    except subprocess.TimeoutExpired as e:
        print(f"QEMU ran longer then {args.timeout} seconds.")
        return
    except Exception as e:
        if swtpm_proc is not None:
            swtpm_proc.kill()
        raise e

    if swtpm_proc is not None:
        swtpm_proc.kill()


def update_firmware():
    # Check if this is the newest version fore awareness.
    latest_version = get_latest_version()
    if args.version == "latest":
        args.version = latest_version
    elif args.version != latest_version:
        print("#############################################################")
        print(f"NOTE: A newer version of firmware available! {latest_version}")
        print(f"use \"--version latest\" to download the latest version")
        print("#############################################################\n")

    #
    # Updates the firmware to the following configuration.
    #     <root>/<arch>/<platform>/<build_toolchain>/<files>
    #
    print(f"Updating firmware to version {args.version}...")

    if not os.path.exists(args.firmwaredir):
        os.makedirs(args.firmwaredir)

    fw_info_list = [["QemuQ35", "x64", True],
                    ["QemuQ35.NoSmm", "x64", False],
                    ["QemuSbsa", "aarch64", True]]

    for fw_info in fw_info_list:
        build_type = "DEBUG" if args.debugfw and fw_info[2] else "RELEASE"
        url = f"https://github.com/microsoft/mu_tiano_platforms/releases/download/v{args.version}/Mu.{fw_info[0]}.FW.{build_type}-{args.version}.zip"
        zip_path = f"{args.firmwaredir}/{fw_info[0]}.zip"

        print(f"Downloading {fw_info[0]}")
        urllib.request.urlretrieve(url, zip_path)
        print(f"Unzipping {fw_info[0]}")
        unzip_path = f"{args.firmwaredir}/{fw_info[1]}/{fw_info[0]}/"
        shutil.rmtree(unzip_path, ignore_errors=True)
        with zipfile.ZipFile(zip_path, "r") as zip:
            zip.extractall(unzip_path)
        os.remove(zip_path)

    print("Done.")


def get_latest_version():
    response = requests.get(
        "https://api.github.com/repos/microsoft/mu_tiano_platforms/releases/latest")

    version = response.json()["name"]
    assert version[0] == 'v'
    return version[1:]


try:
    main()
except KeyboardInterrupt as e:
    sys.stdout.write("\n")
    pass
