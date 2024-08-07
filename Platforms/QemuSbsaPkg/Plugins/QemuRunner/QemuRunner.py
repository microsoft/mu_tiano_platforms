##
# This plugin runs the QEMU command and monitors for asserts.
# It can also possibly run tests and parse the results
#
# Copyright (c) Microsoft Corporation
# SPDX-License-Identifier: BSD-2-Clause-Patent
##

import logging
import io
import os
import re
import datetime
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

        # Use a provided QEMU path. Otherwise use what is provided through the extdep
        executable = env.GetValue("QEMU_PATH", None)
        if not executable:
            executable = str(Path(env.GetValue("QEMU_DIR", ''),"qemu-system-aarch64"))

        qemu_version = QemuRunner.QueryQemuVersion(executable)

        # turn off network
        args = "-net none"

        # If we are using the QEMU external dependency, we need to tell it
        # where to look for roms
        if not env.GetValue("QEMU_PATH") and env.GetValue("QEMU_DIR"):
            args += f" -L {str(Path(env.GetValue('QEMU_DIR'), 'share'))}"

        # Mount disk with either startup.nsh or OS image
        path_to_os = env.GetValue("PATH_TO_OS")
        if path_to_os is not None:
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
        elif os.path.isfile(VirtualDrive):
            args += f" -drive file={VirtualDrive},if=virtio"
        elif os.path.isdir(VirtualDrive):
            args += f" -drive file=fat:rw:{VirtualDrive},format=raw,media=disk"
        else:
            logging.critical("Virtual Drive Path Invalid")

        if path_to_os is not None:
            args += " -m 8192"
        else:
            args += " -m 2048"

        args += " -machine sbsa-ref" #,accel=(tcg|kvm)"
        args += " -cpu max,sme=off,sve=off"
        if env.GetBuildValue ("QEMU_CORE_NUM") is not None:
          args += " -smp " + env.GetBuildValue ("QEMU_CORE_NUM")
        args += " -global driver=cfi.pflash01,property=secure,value=on"
        args += " -drive if=pflash,format=raw,unit=0,file=" + \
            os.path.join(OutputPath_FV, "SECURE_FLASH0.fd")

        code_fd = os.path.join(OutputPath_FV, "QEMU_EFI.fd")
        args += " -drive if=pflash,format=raw,unit=1,file=" + \
                code_fd + ",readonly=on"

        # Add XHCI USB controller and mouse
        args += " -device qemu-xhci,id=usb"
        args += " -device usb-tablet,id=input0,bus=usb.0,port=1"  # add a usb mouse
        args += " -device usb-kbd,id=input1,bus=usb.0,port=2"     # add a usb keyboard

        creation_time = Path(code_fd).stat().st_ctime
        creation_datetime = datetime.datetime.fromtimestamp(creation_time)
        creation_date = creation_datetime.strftime("%m/%d/%Y")

        args += f" -smbios type=0,vendor=\"Project Mu\",version=\"mu_tiano_platforms-{repo_version}\",date={creation_date},uefi=on"
        args += f" -smbios type=1,manufacturer=Palindrome,product=\"QEMU SBSA\",family=QEMU,version=\"{'.'.join(qemu_version)}\",serial=42-42-42-42"
        args += f" -smbios type=3,manufacturer=Palindrome,serial=42-42-42-42,asset=SBSA,sku=SBSA"

        if (env.GetValue("QEMU_HEADLESS").upper() == "TRUE"):
            args += " -display none"  # no graphics

        # Check for gdb server setting
        gdb_port = env.GetValue("GDB_SERVER")
        if (gdb_port != None):
            logging.log(logging.INFO, "Enabling GDB server at port tcp::" + gdb_port + ".")
            args += " -gdb tcp::" + gdb_port

        # write ConOut messages to telnet localhost port
        serial_port = env.GetValue("SERIAL_PORT")
        if serial_port != None:
            args += " -serial tcp:127.0.0.1:" + serial_port + ",server,nowait"
        else:
            # write messages to stdio
            args += " -serial stdio"

        # Connect the debug monitor to a telnet localhost port
        monitor_port = env.GetValue("MONITOR_PORT")
        if monitor_port is not None:
            args += " -monitor tcp:127.0.0.1:" + monitor_port + ",server,nowait"

        ## TODO: Save the console mode. The original issue comes from: https://gitlab.com/qemu-project/qemu/-/issues/1674
        if os.name == 'nt' and qemu_version[0] >= '8':
            import win32console
            std_handle = win32console.GetStdHandle(win32console.STD_INPUT_HANDLE)
            try:
                console_mode = std_handle.GetConsoleMode()
            except Exception:
                std_handle = None

        # Run QEMU
        ret = utility_functions.RunCmd(executable, args)

        ## TODO: restore the customized RunCmd once unit tests with asserts are figured out
        if ret == 0xc0000005:
            ret = 0

        ## TODO: remove this once we upgrade to newer QEMU
        if ret == 0x8B and qemu_version[0] == '4':
            # QEMU v4 will return segmentation fault when shutting down.
            # Tested same FDs on QEMU 6 and 7, not observing the same.
            ret = 0

        if os.name == 'nt' and qemu_version[0] >= '8' and std_handle is not None:
            # Restore the console mode for Windows on QEMU v8+.
            std_handle.SetConsoleMode(console_mode)
        elif os.name != 'nt':
            # Linux version of QEMU will mess with the print if its run failed, let's just restore it anyway
            utility_functions.RunCmd('stty', 'sane', capture=False)

        return ret
