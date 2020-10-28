# @file
# Script to Build QemuQ35 Mu UEFI firmware
#
# Copyright (c) Microsoft Corporation.
# SPDX-License-Identifier: BSD-2-Clause-Patent
##
import os
import logging
import io
import shutil
import glob
import time

from edk2toolext.environment import shell_environment
from edk2toolext.environment.uefi_build import UefiBuilder
from edk2toolext.invocables.edk2_platform_build import BuildSettingsManager
from edk2toolext.invocables.edk2_setup import SetupSettingsManager, RequiredSubmodule
from edk2toolext.invocables.edk2_update import UpdateSettingsManager
from edk2toolext.invocables.edk2_pr_eval import PrEvalSettingsManager
from edk2toollib.utility_functions import RunCmd


    # ####################################################################################### #
    #                                Common Configuration                                     #
    # ####################################################################################### #
class CommonPlatform():
    ''' Common settings for this platform.  Define static data here and use
        for the different parts of stuart
    '''
    PackagesSupported = ("QemuQ35Pkg",)
    ArchSupported = ("IA32", "X64")
    TargetsSupported = ("DEBUG", "RELEASE", "NOOPT")
    Scopes = ('qemuq35', 'edk2-build', 'cibuild')
    WorkspaceRoot = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    PackagesPath = ("Platforms", "MU_BASECORE", "Common/MU", "Common/MU_TIANO", "Common/MU_OEM_SAMPLE")


    # ####################################################################################### #
    #                         Configuration for Update & Setup                                #
    # ####################################################################################### #
class SettingsManager(UpdateSettingsManager, SetupSettingsManager, PrEvalSettingsManager):

    def GetPackagesSupported(self):
        ''' return iterable of edk2 packages supported by this build.
        These should be edk2 workspace relative paths '''
        return CommonPlatform.PackagesSupported

    def GetArchitecturesSupported(self):
        ''' return iterable of edk2 architectures supported by this build '''
        return CommonPlatform.ArchSupported

    def GetTargetsSupported(self):
        ''' return iterable of edk2 target tags supported by this build '''
        return CommonPlatform.TargetsSupported

    def GetRequiredSubmodules(self):
        ''' return iterable containing RequiredSubmodule objects.
        If no RequiredSubmodules return an empty iterable
        '''
        rs = []

        # To avoid maintenance of this file for every new submodule
        # lets just parse the .gitmodules and add each if not already in list.
        # The GetRequiredSubmodules is designed to allow a build to optimize
        # the desired submodules but it isn't necessary for this repository.
        result = io.StringIO()
        ret = RunCmd("git", "config --file .gitmodules --get-regexp path",
                     workingdir=self.GetWorkspaceRoot(), outstream=result)
        # Cmd output is expected to look like:
        # submodule.CryptoPkg/Library/OpensslLib/openssl.path CryptoPkg/Library/OpensslLib/openssl
        # submodule.SoftFloat.path ArmPkg/Library/ArmSoftFloatLib/berkeley-softfloat-3
        if ret == 0:
            for line in result.getvalue().splitlines():
                _, _, path = line.partition(" ")
                if path is not None:
                    if path not in [x.path for x in rs]:
                        # add it with recursive since we don't know
                        rs.append(RequiredSubmodule(path, True))
        return rs

    def SetArchitectures(self, list_of_requested_architectures):
        ''' Confirm the requests architecture list is valid and configure SettingsManager
        to run only the requested architectures.

        Raise Exception if a list_of_requested_architectures is not supported
        '''
        unsupported = set(list_of_requested_architectures) - \
            set(self.GetArchitecturesSupported())
        if(len(unsupported) > 0):
            errorString = (
                "Unsupported Architecture Requested: " + " ".join(unsupported))
            logging.critical( errorString )
            raise Exception( errorString )
        self.ActualArchitectures = list_of_requested_architectures

    def GetWorkspaceRoot(self):
        ''' get WorkspacePath '''
        return CommonPlatform.WorkspaceRoot

    def GetActiveScopes(self):
        ''' return tuple containing scopes that should be active for this process '''
        return CommonPlatform.Scopes

    def FilterPackagesToTest(self, changedFilesList: list, potentialPackagesList: list) -> list:
        ''' Filter other cases that this package should be built
        based on changed files. This should cover things that can't
        be detected as dependencies. '''
        build_these_packages = []
        possible_packages = potentialPackagesList.copy()
        for f in changedFilesList:
            # BaseTools files that might change the build
            if "BaseTools" in f:
                if os.path.splitext(f) not in [".txt", ".md"]:
                    build_these_packages = possible_packages
                    break

            # if the azure pipeline platform template file changed
            if "platform-build-run-steps.yml" in f:
                build_these_packages = possible_packages
                break

        return build_these_packages

    def GetPlatformDscAndConfig(self) -> tuple:
        ''' If a platform desires to provide its DSC then Policy 4 will evaluate if
        any of the changes will be built in the dsc.

        The tuple should be (<workspace relative path to dsc file>, <input dictionary of dsc key value pairs>)
        '''
        return ("QemuQ35Pkg/QemuQ35Pkg.dsc", {})

    def GetName(self):
        return "QemuQ35"

    def GetPackagesPath(self):
        ''' Return a list of paths that should be mapped as edk2 PackagesPath '''
        return CommonPlatform.PackagesPath

    # ####################################################################################### #
    #                         Actual Configuration for Platform Build                         #
    # ####################################################################################### #
class PlatformBuilder( UefiBuilder, BuildSettingsManager):
    def __init__(self):
        UefiBuilder.__init__(self)

    def AddCommandLineOptions(self, parserObj):
        ''' Add command line options to the argparser '''

        # In an effort to support common server based builds this parameter is added.  It is
        # checked for correctness but is never uses as this platform only supports a single set of
        # architectures.
        parserObj.add_argument('-a', "--arch", dest="build_arch", type=str, default="IA32,X64",
            help="Optional - CSV of architecture to build.  IA32,X64 will use IA32 for PEI and "
            "X64 for DXE and is the only valid option for this platform.")

    def RetrieveCommandLineOptions(self, args):
        '''  Retrieve command line options from the argparser '''
        if args.build_arch.upper() != "IA32,X64":
            raise Exception("Invalid Arch Specified.  Please see comments in PlatformBuild.py::PlatformBuilder::AddCommandLineOptions")


    def GetWorkspaceRoot(self):
        ''' get WorkspacePath '''
        return CommonPlatform.WorkspaceRoot

    def GetPackagesPath(self):
        ''' Return a list of workspace relative paths that should be mapped as edk2 PackagesPath '''
        return CommonPlatform.PackagesPath

    def GetActiveScopes(self):
        ''' return tuple containing scopes that should be active for this process '''
        return CommonPlatform.Scopes

    def GetName(self):
        ''' Get the name of the repo, platform, or product being build '''
        ''' Used for naming the log file, among others '''
        # check the startup nsh flag and if set then rename the log file.
        # this helps in CI so we don't overwrite the build log since running
        # uses the stuart_build command.
        if(shell_environment.GetBuildVars().GetValue("MAKE_STARTUP_NSH", "FALSE") == "TRUE"):
            return "QemuQ35Pkg_With_Run"
        return "QemuQ35Pkg"

    def GetLoggingLevel(self, loggerType):
        ''' Get the logging level for a given type
        base == lowest logging level supported
        con  == Screen logging
        txt  == plain text file logging
        md   == markdown file logging
        '''
        return logging.INFO
        return super().GetLoggingLevel(loggerType)

    def SetPlatformEnv(self):
        logging.debug("PlatformBuilder SetPlatformEnv")
        self.env.SetValue("PRODUCT_NAME", "QemuQ35", "Platform Hardcoded")
        self.env.SetValue("ACTIVE_PLATFORM", "QemuQ35Pkg/QemuQ35Pkg.dsc", "Platform Hardcoded")
        self.env.SetValue("TARGET_ARCH", "IA32 X64", "Platform Hardcoded")
        self.env.SetValue("MAKE_STARTUP_NSH", "FALSE", "Default to false")
        self.env.SetValue("RUN_UNIT_TESTS", "FALSE", "Default to false")
        self.env.SetValue("QEMU_HEADLESS", "FALSE", "Default to false")
        # needed to make FV size build report happy
        self.env.SetValue("BLD_*_BUILDID_STRING", "Unknown", "Default")
        # Default turn on build reporting.
        self.env.SetValue("BUILDREPORTING", "TRUE", "Enabling build report")
        self.env.SetValue("BUILDREPORT_TYPES", "PCD DEPEX FLASH BUILD_FLAGS LIBRARY FIXED_ADDRESS HASH", "Setting build report types")

        return 0

    def PlatformPreBuild(self):
        return 0

    def PlatformPostBuild(self):
        return 0

    def FlashRomImage(self):
        #Make or Mount virtual drive - Allow caller to override path otherwise use default
        DefaultVirtualDrivePath = os.path.join(self.env.GetValue("BUILD_OUTPUT_BASE"), "VirtualDrive.vhd")
        VirtualDrivePath = self.env.GetValue("VIRTUAL_DRIVE_PATH", DefaultVirtualDrivePath)
        self.env.SetValue("VIRTUAL_DRIVE_PATH", VirtualDrivePath, "Set Virtual Drive path in case not set")

        HostMountDriveLetter = self.env.GetValue("HOST_MOUNT_DRIVE_LETTER", "m")
        VirtualDrive = VirtualDriveManager(VirtualDrivePath, HostMountDriveLetter)

        if self.env.GetValue("KEEP_EXISTING_VIRTUAL_DRIVE_CONTENTS", "false").upper() == "TRUE":
            logging.info("Leaving the Virtual Drive as requested.  Be aware this can impact your unit tests results")
        else:
            if os.path.isfile(VirtualDrivePath):
                os.remove(VirtualDrivePath)
            VirtualDrive.MakeDrive()

        startupnsh = StartUpScriptManager()

        should_run_unit_tests = (self.env.GetValue("RUN_UNIT_TESTS").upper() == "TRUE")
        if should_run_unit_tests:
            ut = UnitTestSupport(os.path.join(self.env.GetValue("BUILD_OUTPUT_BASE"), "X64"))
            ut.set_glob_csv(self.env.GetValue("STARTUP_GLOB_CSV", "*Test*.efi"))
            ut.find_tests()
            ut.copy_tests_to_virtual_drive(VirtualDrive, startupnsh)

        nshpath =  os.path.join(self.env.GetValue("BUILD_OUTPUT_BASE"), "startup.nsh")
        startupnsh.WriteOut(nshpath)
        VirtualDrive.AddFile(nshpath)
        VirtualDrive.DismountDrive()
        ret = self.Helper.QemuRun(self.env)

        # do stuff with unit test results here
        return 0

class UnitTestSupport(object):

    def __init__(self, host_efi_build_output_path: os.PathLike):
        self.test_list = []
        self._globlist = []
        self.host_efi_path = host_efi_build_output_path

    def set_glob_csv(self, csv_string):
        self._globlist = csv_string.split(",")

    def find_tests(self):
        test_list = []
        for globpattern in self._globlist:
            test_list.extend(glob.glob(os.path.join(self.host_efi_path, globpattern)))
        self.test_list = list(set(test_list))

    def copy_tests_to_virtual_drive(self, virtualdrive, nshfile):
        for test in self.test_list:
            virtualdrive.AddFile(test)
            nshfile.AddLine(os.path.basename(test))

    def __tood(self):
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
                try:
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
                except Exception as ex:
                    logging.error("Exception trying to read xml." + str(ex))
                    failure_count += 1

            else:
                logging.warning("%s Test Failed - No Results File" % os.path.basename(unit_test))
                failure_count += 1
        return failure_count



class VirtualDriveManager(object):

    def __init__(self, vhd_path:os.PathLike, host_drive_letter:str):
        self.path_to_vhd = os.path.abspath(vhd_path)
        self._host_drive_letter = host_drive_letter
        self.host_path = os.path.join(self._host_drive_letter + ':', "/")
        self._host_mounted = False

    def __del__(self):
        if self._host_mounted:
            self.DismountDrive()

    def MakeDrive(self, size: int=100):
        scriptfile = os.path.join(os.path.dirname(self.path_to_vhd), "vhd_script.tmp")
        with open(scriptfile, "w") as sf:
            sf.write(f"create vdisk file={self.path_to_vhd} maximum={size}\n")
            sf.write(f"select vdisk file={self.path_to_vhd}\n")
            sf.write(f"convert gpt\n")
            sf.write(f"create partition primary\n")
            sf.write(f'format fs=fat32 label="install" quick\n')
            sf.write(f'assign letter={self._host_drive_letter}\n')
        ret = RunCmd("diskpart", f" /S {scriptfile}")
        time.sleep(15)  # sleep 15 seconds
        os.remove(scriptfile)
        self._host_mounted = True
        return ret

    def MountDrive(self):
        if not self._host_mounted:
            if not os.path.isfile:
                raise Exception("Virtual Drive file does not exist")

            scriptfile = os.path.join(os.path.dirname(self.path_to_vhd), "vhd_script.tmp")
            with open(scriptfile, "w") as sf:
                sf.write(f"select vdisk file={self.path_to_vhd}\n")
                sf.write(f'assign letter={self._host_drive_letter}\n')
            RunCmd("diskpart", f" /S {scriptfile}")
            #time.sleep(15)  # sleep 15 seconds
            os.remove(scriptfile)
            self._host_mounted = True

    def DismountDrive(self):
        if self._host_mounted:
            scriptfile = os.path.join(os.path.dirname(self.path_to_vhd), "vhd_script.tmp")
            with open(scriptfile, "w") as sf:
                sf.write(f"select vdisk file={self.path_to_vhd}\n")
                sf.write(f"detach vdisk\n")
            RunCmd("diskpart", f" /S {scriptfile}")
            time.sleep(15)  # sleep 15 seconds
            os.remove(scriptfile)
            self._host_mounted = False

    def AddFile(self, HostFilePath):
        if not self._host_mounted:
            self.MountDrive()

        shutil.copy(HostFilePath, self.host_path)

    def GetFileContent(self, VirtualFilePath):
        if not self._host_mounted:
            self.MountDrive()
        with open(os.path.join(self.host_path, VirtualFilePath), "rb") as f:
            return f.read()


class StartUpScriptManager(object):

    FS_FINDER_SCRIPT = r'''
#!/bin/nsh
echo -off
for %a run (0 10)
    if exist fs%a:\{first_file} then
        fs%a:
        goto FOUND_IT
    endif
endfor

:FOUND_IT
'''

    def __init__(self):
        self._use_fs_finder = False
        self._lines = []
        self._add_shutdown_to_end = True

    def WriteOut(self, host_file_path):
        with open(host_file_path, "wb") as nsh:
            if self._use_fs_finder:
                this_file = os.path.basename(host_file_path)
                nsh.write(StartUpScriptManager.FS_FINDER_SCRIPT.format(first_file=this_file))

            for l in self._lines:
                nsh.write(l + "\n")

            if self._add_shutdown_to_end:
                nsh.write("reset -s\n")

    def AddLine(self, line):
        self._lines.append(line.strip())
        self._use_fs_finder = True



if __name__ == "__main__":
    import argparse
    import sys
    from edk2toolext.invocables.edk2_update import Edk2Update
    from edk2toolext.invocables.edk2_setup import Edk2PlatformSetup
    from edk2toolext.invocables.edk2_platform_build import Edk2PlatformBuild
    print("Invoking Stuart")
    print("     ) _     _")
    print("    ( (^)-~-(^)")
    print("__,-.\_( 0 0 )__,-.___")
    print("  'W'   \   /   'W'")
    print("         >o<")
    SCRIPT_PATH = os.path.relpath(__file__)
    parser = argparse.ArgumentParser(add_help=False)
    parse_group = parser.add_mutually_exclusive_group()
    parse_group.add_argument("--update", "--UPDATE",
                             action='store_true', help="Invokes stuart_update")
    parse_group.add_argument("--setup", "--SETUP",
                             action='store_true', help="Invokes stuart_setup")
    args, remaining = parser.parse_known_args()
    new_args = ["stuart", "-c", SCRIPT_PATH]
    new_args = new_args + remaining
    sys.argv = new_args
    if args.setup:
        print("Running stuart_setup -c " + SCRIPT_PATH)
        Edk2PlatformSetup().Invoke()
    elif args.update:
        print("Running stuart_update -c " + SCRIPT_PATH)
        Edk2Update().Invoke()
    else:
        print("Running stuart_build -c " + SCRIPT_PATH)
        Edk2PlatformBuild().Invoke()