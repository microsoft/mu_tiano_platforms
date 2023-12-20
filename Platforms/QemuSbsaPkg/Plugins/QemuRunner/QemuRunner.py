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
import threading
import datetime
import subprocess
from pathlib import Path
from edk2toolext.environment import plugin_manager
from edk2toolext.environment.plugintypes import uefi_helper_plugin
from edk2toollib import utility_functions
from edk2toollib.uefi.edk2.parsers.dsc_parser import DscParser
from edk2toollib.uefi.edk2.parsers.inf_parser import InfParser
from edk2toolext.environment.multiple_workspace import MultipleWorkspace

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
        executable = env.GetValue("QEMU_PATH", "qemu-system-aarch64")

        qemu_version = QemuRunner.QueryQemuVersion(executable)

        # turn off network
        args = "-net none"

        # Mount disk with either startup.nsh or OS image
        path_to_os = env.GetValue("PATH_TO_OS")
        if path_to_os is not None:
            file_extension = Path(path_to_os).suffix.lower().replace('"', '')

            storage_format = {
                ".vhd": "raw",
                ".qcow2": "qcow2"
            }.get(file_extension, None)

            if storage_format is None:
                raise Exception(f"Unknown OS storage type: {path_to_os}")

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
        args += " -cpu max"
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

        # Run QEMU
        #ret = QemuRunner.RunCmd(executable, args,  thread_target=QemuRunner.QemuCmdReader)
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

    ####
    # Helper functions for running commands from the shell in python environment
    # Don't use directly
    #
    # process output stream and write to log.
    # part of the threading pattern.
    #
    #  http://stackoverflow.com/questions/19423008/logged-subprocess-communicate
    ####
    @staticmethod
    def QemuCmdReader(filepath, outstream, stream, logging_level=logging.INFO):
        f = None
        # open file if caller provided path
        error_found = False
        if(filepath):
            f = open(filepath, "w")
        while True:
            try:
                s = stream.readline().decode()
                ss = s.rstrip() # string stripped
            except UnicodeDecodeError as e:
                logging.error(str(e))
            if not s:
                break
            if(f is not None):
                # write to file if caller provided file
                f.write(ss)
                f.write("\n")
            if(outstream is not None):
                # write to stream object if caller provided object
                outstream.write(ss)
                f.write("\n")
            logging.log(logging_level, ss)
            if s.startswith("ASSERT "):
                message = "ASSERT DETECTED, killing QEMU process: " + ss
                logging.error(message)
                if (outstream is not None):
                    outstream.write(message)
                if (f is not None):
                    f.write(message)
                error_found = True
                break
        stream.close()
        if(f is not None):
            f.close()
        return None if not error_found else 1

####
# Class to support running commands from the shell in a python environment.
# Don't use directly.
#
# PropagatingThread copied from sample here:
# https://stackoverflow.com/questions/2829329/catch-a-threads-exception-in-the-caller-thread-in-python
####
class PropagatingThread(threading.Thread):
    def run(self):
        self.exc = None
        self.ret = None
        try:
            if hasattr(self, '_Thread__target'):
                # Thread uses name mangling prior to Python 3.
                self.ret = self._Thread__target(*self._Thread__args, **self._Thread__kwargs)
            else:
                self.ret = self._target(*self._args, **self._kwargs)
        except SystemExit as e:
            self.ret = e.code
        except BaseException as e:
            self.exc = e

    def join(self, timeout=0.5):
        ''' timeout is the number of seconds to timeout '''
        super(PropagatingThread, self).join(timeout)
        if self.exc:
            raise self.exc
        return self.ret
