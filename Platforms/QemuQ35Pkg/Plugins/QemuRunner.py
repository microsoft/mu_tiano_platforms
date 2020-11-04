##
# This plugin runs the QEMU command, monitoring for asserts
#
# Copyright (c) Microsoft Corporation
#
##

import logging
import os
import sys
import time
import shutil
import glob
import threading
import datetime
import subprocess
import xml.etree.ElementTree
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
    def Runner(env):
        ''' Runs QEMU '''
        VirtualDrive = os.path.join(env.GetValue("BUILD_OUTPUT_BASE"), "VirtualDrive")
        if os.path.isdir(VirtualDrive):
            if env.GetValue("STARTUP_NSH_DIRTY", "false").upper() == "TRUE":
                logging.info("Leaving the Virtual Drive Dirty as requested.  Be aware this can impact your unit tests results")
            else:
                shutil.rmtree(VirtualDrive)

        os.makedirs(VirtualDrive, exist_ok=True)
        OutputPath_FV = os.path.join(env.GetValue("BUILD_OUTPUT_BASE"), "FV")

        # Copy required tests.
        TestEfiOutputPath = os.path.join(
            env.GetValue("BUILD_OUTPUT_BASE"), "X64")
        shutil.copy(os.path.join(TestEfiOutputPath,
                                 "VarPolicyUnitTestApp.efi"), VirtualDrive)

        # Check if QEMU is on the path, if not find it
        executable = "qemu-system-x86_64"

        # write messages to stdio
        args = "-debugcon stdio"
        # debug messages out thru virtual io port
        args += " -global isa-debugcon.iobase=0x402"
        # Turn off S3 support
        args += " -global ICH9-LPC.disable_s3=1"
        # turn off network
        args += " -net none"
        # Mount disk with startup.nsh
        args += f" -drive file=fat:rw:{VirtualDrive},format=raw,media=disk"

        args += " -machine q35,smm=on" #,accel=(tcg|kvm)"
        args += " -m 2048"
        #args += " -smp ..."
        args += " -global driver=cfi.pflash01,property=secure,value=on"
        args += " -drive if=pflash,format=raw,unit=0,file=" + \
            os.path.join(OutputPath_FV, "QEMUQ35_CODE.fd") + ",readonly=on"
        args += " -drive if=pflash,format=raw,unit=1,file=" + \
            os.path.join(OutputPath_FV, "QEMUQ35_VARS.fd")

        # Add XHCI USB controller and mouse
        args += " -device qemu-xhci,id=usb"
        args += " -device usb-mouse,id=input0,bus=usb.0,port=1"  # add a usb mouse
        #args += " -device usb-kbd,id=input1,bus=usb.0,port=2"    # add a usb keyboar

        if (env.GetValue("QEMU_HEADLESS").upper() == "TRUE"):
            args += " -display none"  # no graphics
        else:
            args += " -vga cirrus" #std is what the default is

        # Find the unit tests and copy them to the virtual drive
        unit_tests = QemuRunner.collect_unit_tests(env)
        for unit_test in unit_tests:
            logging.debug("Copying " + unit_test)
            shutil.copy(unit_test, VirtualDrive)

        # Setup Startup.nsh if needed
        should_run_unit_tests = (env.GetValue("RUN_UNIT_TESTS").upper() == "TRUE")
        if (env.GetValue("MAKE_STARTUP_NSH").upper() == "TRUE") or should_run_unit_tests:
            f = open(os.path.join(VirtualDrive, "startup.nsh"), "w")
            if should_run_unit_tests:
                # Write out script to find the filesystem - this is to avoid hardcoding fs0:
                script = '';
                script += '#!/bin/nsh\n'
                #script += 'echo -off\n'
                script += 'for %a run (0 5)\n'
                script += '    if exist fs%a:\*TestApp.efi then\n'
                script += '        fs%a:\n'
                script += '        goto TESTS\n'
                script += '    endif\n'
                script += 'endfor\n'
                script += '\n'
                script += ':TESTS\n'
                for unit_test in unit_tests:
                    script += os.path.basename(unit_test)
                    logging.critical(f"Writing {unit_test} to startup nsh file")
                    script += "\n"
                f.write(script)
            else:
                f.write("# BOOT SUCCESS !!! \n")
            f.write("reset -s\n")
            f.close()
        
        # Run QEMU
        qemu_out_file = os.path.join(env.GetValue("WORKSPACE"), env.GetValue("OUTPUT_DIRECTORY"),f"QEMULOG.txt")
        ret = QemuRunner.RunCmd(executable, args,  outfile=qemu_out_file, thread_target=QemuRunner.QemuCmdReader)
        
        if ret != 0x0 and ret != 0xc0000005:
            #for some reason getting a c0000005 on successful return
            return ret
        # if you didn't do unit tests, don't check for errors
        if not should_run_unit_tests:
            return 0
        #now parse the xml for errors
        failure_count = 0
        logging.info("UnitTest Completed")
        for unit_test in unit_tests:
            xml_result_file = os.path.join(VirtualDrive, os.path.basename(unit_test)[:-4] + "_JUNIT.XML")
            if os.path.isfile(xml_result_file):
                logging.info('\n' + os.path.basename(unit_test))
                root = xml.etree.ElementTree.parse(xml_result_file).getroot()
                for suite in root:
                    logging.info(" ")
                    for case in suite:
                        logging.info('\t\t' + case.attrib['classname'] + " - ")
                        caseresult = "\t\t\tPASS"
                        level = logging.INFO
                        for result in case:
                            if result.tag == 'failure':
                                failure_count += 1
                                level = logging.ERROR
                                caseresult = "\t\tFAIL" + " - " + result.attrib['message']
                        logging.log( level, caseresult)
            else:
                logging.warning("%s Test Failed - No Results File" % os.path.basename(unit_test))
                failure_count += 1
        return failure_count

    @staticmethod
    def collect_unit_tests(env):
        ''' returns a list of absolute paths to unit test EFI's '''
        path = env.GetValue("BUILD_OUTPUT_BASE")
        cp = os.path.join(path, "X64")
        testList = glob.glob(os.path.join(cp, "*TestApp.efi"))
        #testList.extend(glob.glob(os.path.join(cp, "*TestSmm.efi")))
        testList.extend(glob.glob(os.path.join(cp, "*TestShell.efi")))
        return testList

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