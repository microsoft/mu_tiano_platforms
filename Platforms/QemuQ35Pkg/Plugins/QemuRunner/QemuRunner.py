##
# This plugin runs the QEMU command and monitors for asserts.
# It can also possibly run tests and parse the results
#
# Copyright (c) Microsoft Corporation
# SPDX-License-Identifier: BSD-2-Clause-Patent
##

import logging
import os
import datetime
import re
import io
import shutil
from pathlib import Path
from edk2toolext.environment.plugintypes import uefi_helper_plugin
from edk2toollib import utility_functions

class QemuRunner(uefi_helper_plugin.IUefiHelperPlugin):

    def __init__(self):
        self.logger = logging.getLogger(__name__)

    def RegisterHelpers(self, obj):
        fp = os.path.abspath(__file__)
        obj.Register("QemuRun", QemuRunner.Runner, fp)
        return 0

    @staticmethod
    # raw helper function to extract version number from QEMU
    def QueryQemuVersion(exec):
        if exec is None:
            return None

        result = io.StringIO()
        ret = utility_functions.RunCmd(exec, "--version", outstream=result)
        if ret != 0:
            return None

        # expected version string will be "QEMU emulator version maj.min.rev"
        res = result.getvalue()
        ver_str = re.search(r'version\s*([\d.]+)', res).group(1)

        return ver_str.split('.')


    @staticmethod
    def Runner(env):
        ''' Runs QEMU '''
        VirtualDrive = env.GetValue("VIRTUAL_DRIVE_PATH")
        OutputPath_FV = os.path.join(env.GetValue("BUILD_OUTPUT_BASE"), "FV")
        repo_version = env.GetValue("VERSION", "Unknown")

        # Use a provided QEMU path. Default to the system path if not provided.
        executable = env.GetValue("QEMU_PATH", "qemu-system-x86_64")

        # First query the version
        qemu_version = QemuRunner.QueryQemuVersion(executable)

        # write messages to stdio
        args = "-debugcon stdio"
        # debug messages out thru virtual io port
        args += " -global isa-debugcon.iobase=0x402"
        # Turn off S3 support
        args += " -global ICH9-LPC.disable_s3=1"

        if env.GetBuildValue("SMM_ENABLED") is None or env.GetBuildValue("SMM_ENABLED").lower() == "true":
            smm_enabled = "on"
        else:
            smm_enabled = "off"

        accel = ""
        if env.GetValue("QEMU_ACCEL") is not None:
            if env.GetValue("QEMU_ACCEL").lower() == "kvm":
                accel = ",accel=kvm"
            elif env.GetValue("QEMU_ACCEL").lower() == "tcg":
                accel = ",accel=tcg"
            elif env.GetValue("QEMU_ACCEL").lower() == "whpx":
                accel = ",accel=whpx"

        args += " -machine q35,smm=" + smm_enabled + accel
        path_to_os = env.GetValue("PATH_TO_OS")
        if path_to_os is not None:
            # Potentially dealing with big daddy, give it more juice...
            args += " -m 8192"

            file_extension = Path(path_to_os).suffix.lower().replace('"', '')

            storage_format = {
                ".vhd": "raw",
                ".qcow2": "qcow2",
                ".iso": "iso",
            }.get(file_extension, None)

            if storage_format is None:
                raise Exception(f"Unknown OS storage type: {path_to_os}")

            if storage_format == "iso":
                args += f" -cdrom \"{path_to_os}\""
            else:
                args += f" -drive file=\"{path_to_os}\",format={storage_format},if=none,id=os_nvme"
                args += " -device nvme,serial=nvme-1,drive=os_nvme"
        else:
            args += " -m 2048"

        cpu_model = env.GetValue("CPU_MODEL")
        if cpu_model is None:
            cpu_model = "qemu64"

        logging.log(logging.INFO, "CPU model: " + cpu_model)

        #args += " -cpu qemu64,+rdrand,umip,+smep,+popcnt" # most compatible x64 CPU model + RDRAND + UMIP + SMEP +POPCNT support (not included by default)
        cpu_arg = " -cpu " + cpu_model + ",rdrand=on,umip=on,smep=on,pdpe1gb=on,popcnt=on"
        args += cpu_arg

        if env.GetBuildValue ("QEMU_CORE_NUM") is not None:
            args += " -smp " + env.GetBuildValue ("QEMU_CORE_NUM")
        if smm_enabled == "on":
            args += " -global driver=cfi.pflash01,property=secure,value=on"

        code_fd = os.path.join(OutputPath_FV, "QEMUQ35_CODE.fd")
        args += " -drive if=pflash,format=raw,unit=0,file=" + \
                code_fd + ",readonly=on"

        orig_var_store = os.path.join(OutputPath_FV, "QEMUQ35_VARS.fd")
        dfci_var_store =env.GetValue("DFCI_VAR_STORE")
        if dfci_var_store is not None:
            if not os.path.isfile(dfci_var_store):
                shutil.copy(orig_var_store, dfci_var_store)
            use_this_varstore = dfci_var_store
        else:
            use_this_varstore = orig_var_store
        args += " -drive if=pflash,format=raw,unit=1,file=" + use_this_varstore

        # Add XHCI USB controller and mouse
        args += " -device qemu-xhci,id=usb"
        args += " -device usb-tablet,id=input0,bus=usb.0,port=1"  # add a usb mouse
        #args += " -device usb-kbd,id=input1,bus=usb.0,port=2"    # add a usb keyboard

        dfci_files = env.GetValue("DFCI_FILES")
        if dfci_files is not None:
            args += f" -drive file=fat:rw:{dfci_files},format=raw,media=disk,if=none,id=dfci_disk"
            args += " -device usb-storage,bus=usb.0,drive=dfci_disk"

        install_files = env.GetValue("INSTALL_FILES")
        if install_files is not None:
            args += f" -drive file={install_files},format=raw,media=disk,if=none,id=install_disk"
            args += " -device usb-storage,bus=usb.0,drive=install_disk"

        boot_selection = ''
        boot_to_front_page = env.GetValue("BOOT_TO_FRONT_PAGE")
        if boot_to_front_page is not None:
            if (boot_to_front_page.upper() == "TRUE"):
                boot_selection += ",version=Vol+"

        alt_boot_enable = env.GetValue("ALT_BOOT_ENABLE")
        if alt_boot_enable is not None:
            if alt_boot_enable.upper() == "TRUE":
                boot_selection += ",version=Vol-"

        # If DFCI_VAR_STORE is enabled, don't enable the Virtual Drive, and enable the network
        dfci_var_store = env.GetValue("DFCI_VAR_STORE")
        if dfci_var_store is None:
            # turn off network
            args += " -net none"
            # Mount disk with startup.nsh
            if os.path.isfile(VirtualDrive):
                args += f" -drive file={VirtualDrive},if=virtio"
            elif os.path.isdir(VirtualDrive):
                args += f" -drive file=fat:rw:{VirtualDrive},format=raw,media=disk"
            else:
                logging.critical("Virtual Drive Path Invalid")
        else:
            if boot_to_front_page is None:
                # Booting to Windows, use a PCI nic
                args += " -device e1000,netdev=net0"
            else:
                # Booting to UEFI, use virtio-net-pci
                args += " -device virtio-net-pci,netdev=net0"

            # forward ports for robotframework 8270 and 8271
            args += " -netdev user,id=net0,hostfwd=tcp::8270-:8270,hostfwd=tcp::8271-:8271"

        creation_time = Path(code_fd).stat().st_ctime
        creation_datetime = datetime.datetime.fromtimestamp(creation_time)
        creation_date = creation_datetime.strftime("%m/%d/%Y")

        args += f" -smbios type=0,vendor=\"Project Mu\",version=\"mu_tiano_platforms-{repo_version}\",date={creation_date},uefi=on"
        args += f" -smbios type=1,manufacturer=Palindrome,product=\"QEMU Q35\",family=QEMU,version=\"{'.'.join(qemu_version)}\",serial=42-42-42-42,uuid=9de555c0-05d7-4aa1-84ab-bb511e3a8bef"
        args += f" -smbios type=3,manufacturer=Palindrome,serial=40-41-42-43{boot_selection}"

        # TPM in Linux
        tpm_dev = env.GetValue("TPM_DEV")
        if tpm_dev is not None:
            args += f" -chardev socket,id=chrtpm,path={tpm_dev}"
            args += " -tpmdev emulator,id=tpm0,chardev=chrtpm"
            args += " -device tpm-tis,tpmdev=tpm0"

        if (env.GetValue("QEMU_HEADLESS").upper() == "TRUE"):
            args += " -display none"  # no graphics
        else:
            args += " -vga cirrus" #std is what the default is

        # Check for gdb server setting
        gdb_port = env.GetValue("GDB_SERVER")
        if (gdb_port != None):
            logging.log(logging.INFO, "Enabling GDB server at port tcp::" + gdb_port + ".")
            args += " -gdb tcp::" + gdb_port

        # write ConOut messages to telnet localhost port
        serial_port = env.GetValue("SERIAL_PORT")
        if serial_port != None:
            args += " -serial tcp:127.0.0.1:" + serial_port + ",server,nowait"

        # Connect the debug monitor to a telnet localhost port
        monitor_port = env.GetValue("MONITOR_PORT")
        if monitor_port is not None:
            args += " -monitor tcp:127.0.0.1:" + monitor_port + ",server,nowait"

        # Run QEMU
        ret = utility_functions.RunCmd(executable, args)
        if ret != 0 and os.name != 'nt':
            # Linux version of QEMU will mess with the print if its run failed, this is to restore it
            utility_functions.RunCmd ('stty', 'echo')

        ## TODO: restore the customized RunCmd once unit tests with asserts are figured out
        if ret == 0xc0000005:
            ret = 0

        ## TODO: remove this once we upgrade to newer QEMU
        if ret == 0x8B and qemu_version[0] == '4':
            # QEMU v4 will return segmentation fault when shutting down.
            # Tested same FDs on QEMU 6 and 7, not observing the same.
            ret = 0

        return ret
