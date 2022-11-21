##
# This plugin runs the QEMU command and monitors for asserts.
# It can also possibly run tests and parse the results
#
# Copyright (c) Microsoft Corporation
# SPDX-License-Identifier: BSD-2-Clause-Patent
##

import logging
import os
import sys
import time
import threading
import datetime
import subprocess
import re
import io
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

        # Check if QEMU is on the path, if not find it
        executable = "qemu-system-x86_64"

        # First query the version
        ver = QemuRunner.QueryQemuVersion(executable)

        # write messages to stdio
        args = "-debugcon stdio"
        # debug messages out thru virtual io port
        args += " -global isa-debugcon.iobase=0x402"
        # Turn off S3 support
        args += " -global ICH9-LPC.disable_s3=1"
        # turn off network
        args += " -net none"
        # Mount disk with startup.nsh
        if os.path.isfile(VirtualDrive):
            args += f" -hdd {VirtualDrive}"
        elif os.path.isdir(VirtualDrive):
            args += f" -drive file=fat:rw:{VirtualDrive},format=raw,media=disk"
        else:
            logging.critical("Virtual Drive Path Invalid")

        args += " -machine q35,smm=on" #,accel=(tcg|kvm)"
        if env.GetValue("PATH_TO_OS") is not None:
            # Potentially dealing with big daddy, give it more juice...
            args += " -m 8192"
            args += " -hda " + env.GetValue("PATH_TO_OS")
        else:
            args += " -m 2048"
        args += " -cpu qemu64,+rdrand,umip,+smep" # most compatible x64 CPU model + RDRAND + UMIP + SMEP support (not included by default)
        if env.GetBuildValue ("QEMU_CORE_NUM") is not None:
          args += " -smp " + env.GetBuildValue ("QEMU_CORE_NUM")
        args += " -global driver=cfi.pflash01,property=secure,value=on"
        args += " -drive if=pflash,format=raw,unit=0,file=" + \
            os.path.join(OutputPath_FV, "QEMUQ35_CODE.fd") + ",readonly=on"
        args += " -drive if=pflash,format=raw,unit=1,file=" + \
            os.path.join(OutputPath_FV, "QEMUQ35_VARS.fd")

        # Add XHCI USB controller and mouse
        args += " -device qemu-xhci,id=usb"
        args += " -device usb-mouse,id=input0,bus=usb.0,port=1"  # add a usb mouse
        #args += " -device usb-kbd,id=input1,bus=usb.0,port=2"    # add a usb keyboar
        args += " -smbios type=0,vendor=Palindrome,uefi=on -smbios type=1,manufacturer=Palindrome,product=MuQemuQ35,serial=42-42-42-42"

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
        #ret = QemuRunner.RunCmd(executable, args,  thread_target=QemuRunner.QemuCmdReader)
        ret = utility_functions.RunCmd(executable, args)
        if ret != 0 and os.name != 'nt':
            # Linux version of QEMU will mess with the print if its run failed, this is to restore it
            utility_functions.RunCmd ('stty', 'echo')

        ## TODO: restore the customized RunCmd once unit tests with asserts are figured out
        if ret == 0xc0000005:
            ret = 0

        ## TODO: remove this once we upgrade to newer QEMU
        if ret == 0x8B and ver[0] == '4':
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
    # Run a shell command and print the output to the log file
    # This is the public function that should be used to run commands from the shell in python environment
    # @param cmd - command being run, either quoted or not quoted
    # @param parameters - parameters string taken as is
    # @param capture - boolean to determine if caller wants the output captured in any format.
    # @param workingdir - path to set to the working directory before running the command.
    # @param outfile - capture output to file of given path.
    # @param outstream - capture output to a stream.
    # @param environ - shell environment variables dictionary that replaces the one inherited from the
    #                  current process.
    # @param target - a function to call. It must accept four parameters: filepath, outstream, stream, logging_level
    # @param logging_level - log level to log output at.  Default is INFO
    # @param raise_exception_on_nonzero - Setting to true causes exception to be raised if the cmd
    #                                     return code is not zero.
    #
    # @return returncode of called cmd
    ####
    @staticmethod
    def RunCmd(cmd, parameters, capture=True, workingdir=None, outfile=None, outstream=None, environ=None, thread_target=None, logging_level=logging.INFO, raise_exception_on_nonzero=False):
        cmd = cmd.strip('"\'')
        if " " in cmd:
            cmd = '"' + cmd + '"'
        if parameters is not None:
            parameters = parameters.strip()
            cmd += " " + parameters
        if thread_target is None:
            thread_target = utility_functions.reader
        starttime = datetime.datetime.now()
        logging.log(logging_level, "Cmd to run is: " + cmd)
        logging.log(logging_level, "------------------------------------------------")
        logging.log(logging_level, "--------------Cmd Output Starting---------------")
        logging.log(logging_level, "------------------------------------------------")
        wait_delay = 0.5 # we check about every second
        c = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, cwd=workingdir, shell=True, env=environ)
        if(capture):
            thread = PropagatingThread(target=thread_target, args=(outfile, outstream, c.stdout, logging_level))
            thread.start()
            while True:
                try:
                    c.wait(wait_delay)
                except subprocess.TimeoutExpired:
                    # we expect this to throw and this is safe behavior
                    pass
                ret = thread.join(wait_delay)
                if c.poll() is not None or not thread.is_alive() or ret is not None:
                    break
            # if the propagating thread exited but the cmd is still going
            if c.poll() is None and not thread.is_alive():
                logging.log(logging_level,"WARNING: Terminating the process early due to target")
                c.kill()
                if thread.ret != None:
                    c.returncode = thread.ret # force the return code to be non zero
            if c.poll() is None and not thread.is_alive():
                logging.log(logging_level,"WARNING: Killing the process early due to target")
                c.terminate()
                if thread.ret != None:
                    c.returncode = thread.ret # force the return code to be non zero
        else:
            c.wait()
        endtime = datetime.datetime.now()
        delta = endtime - starttime
        endtime_str = "{0[0]:02}:{0[1]:02}".format(divmod(delta.seconds, 60))
        returncode_str = "{0:#010x}".format(c.returncode)
        logging.log(logging_level, "------------------------------------------------")
        logging.log(logging_level, "--------------Cmd Output Finished---------------")
        logging.log(logging_level, "--------- Running Time (mm:ss): " + endtime_str + " ----------")
        logging.log(logging_level, "----------- Return Code: " + returncode_str + " ------------")
        logging.log(logging_level, "------------------------------------------------")

        if raise_exception_on_nonzero and c.returncode != 0:
            raise Exception("{0} failed with Return Code: {1}".format(cmd, returncode_str))
        return c.returncode

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
