# @file
# Script to Build ArmVirtPkg UEFI firmware
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
import xml.etree.ElementTree
import tempfile

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
    PackagesSupported = ("QemuCortexPkg",)
    ArchSupported = ("AARCH64")
    TargetsSupported = ("DEBUG", "RELEASE", "NOOPT")
    Scopes = ('qemucortex', 'edk2-build', 'cibuild')
    WorkspaceRoot = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    PackagesPath = ("Platforms", "MU_BASECORE", "Common/MU", "Common/MU_TIANO", "Common/MU_OEM_SAMPLE", "Common/PRM", "Common/MU_TIANO_ARM")


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
        scopes = CommonPlatform.Scopes
        # scopes += ("gcc_aarch64_linux",) # BEEBE TODO
        return scopes

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
        return ("QemuCortexPkg/QemuCortexPkg.dsc", {})

    def GetName(self):
        return "QemuCortex"

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
        parserObj.add_argument('-a', "--arch", dest="build_arch", type=str, default="AARCH64",
            help="Optional - CSV of architecture to build.  IA32,X64 will use IA32 for PEI and "
            "X64 for DXE and is the only valid option for this platform.") # BEEBE TODO

    def RetrieveCommandLineOptions(self, args):
        '''  Retrieve command line options from the argparser '''
        if args.build_arch.upper() != "AARCH64":
            raise Exception("Invalid Arch Specified.  Please see comments in PlatformBuild.py::PlatformBuilder::AddCommandLineOptions")
        
        shell_environment.GetBuildVars().SetValue(
            "TARGET_ARCH", args.build_arch.upper(), "From CmdLine")

        shell_environment.GetBuildVars().SetValue(
            "ACTIVE_PLATFORM", "QemuCortexPkg/QemuCortexPkg.dsc", "From CmdLine")


    def GetWorkspaceRoot(self):
        ''' get WorkspacePath '''
        return CommonPlatform.WorkspaceRoot

    def GetPackagesPath(self):
        ''' Return a list of workspace relative paths that should be mapped as edk2 PackagesPath '''
        return CommonPlatform.PackagesPath

    def GetActiveScopes(self):
        ''' return tuple containing scopes that should be active for this process '''
        scopes = CommonPlatform.Scopes
        # scopes += ("gcc_aarch64_linux",)
        return scopes

    def GetName(self):
        ''' Get the name of the repo, platform, or product being build '''
        ''' Used for naming the log file, among others '''
        # Check the FlashImage bool and rename the log file if true.
        # Two builds are done during CI: one without the --FlashOnly flag
        # followed by one with the flag. self.FlashImage will be true if the
        # --FlashOnly flag is passed, meaning we will keep separate build and run logs
        if(self.FlashImage):
            return "QemuCortexPkg_Run"
        return "QemuCortexPkg"

    def GetLoggingLevel(self, loggerType):
        ''' Get the logging level for a given type
        base == lowest logging level supported
        con  == Screen logging
        txt  == plain text file logging
        md   == markdown file logging
        '''
        return logging.DEBUG
        return super().GetLoggingLevel(loggerType)

    def SetPlatformEnv(self):
        logging.debug("PlatformBuilder SetPlatformEnv")
        self.env.SetValue("PRODUCT_NAME", "QemuCortex", "Platform Hardcoded")
        self.env.SetValue("ACTIVE_PLATFORM", "QemuCortexPkg/QemuCortexPkg.dsc", "Platform Hardcoded")
        self.env.SetValue("TARGET_ARCH", "AARCH64", "Platform Hardcoded")
        # self.env.SetValue("EMPTY_DRIVE", "FALSE", "Default to false")
        # self.env.SetValue("RUN_TESTS", "FALSE", "Default to false")
        self.env.SetValue("QEMU_HEADLESS", "FALSE", "Default to false")
        # self.env.SetValue("SHUTDOWN_AFTER_RUN", "FALSE", "Default to false")
        # needed to make FV size build report happy
        # self.env.SetValue("BLD_*_BUILDID_STRING", "Unknown", "Default")
        # # Default turn on build reporting.
        self.env.SetValue("BUILDREPORTING", "TRUE", "Enabling build report")
        self.env.SetValue("BUILDREPORT_TYPES", "PCD DEPEX FLASH BUILD_FLAGS LIBRARY FIXED_ADDRESS HASH", "Setting build report types")
        # Include the MFCI test cert by default, override on the commandline with "BLD_*_SHIP_MODE=TRUE" if you want the retail MFCI cert
        # self.env.SetValue("BLD_*_SHIP_MODE", "FALSE", "Default")

        return 0

    def PlatformPreBuild(self):
        return 0

    def PlatformPostBuild(self):
        return 0

    def FlashRomImage(self):

        VirtualDrive = os.path.join(self.env.GetValue(
            "BUILD_OUTPUT_BASE"), "VirtualDrive")
        os.makedirs(VirtualDrive, exist_ok=True)
        OutputPath_FV = os.path.join(
            self.env.GetValue("BUILD_OUTPUT_BASE"), "FV")
        Built_FV = os.path.join(OutputPath_FV, "QEMU_EFI.fd")
    

        # pad fd to 64mb
        with open(Built_FV, "ab") as fvfile:
            fvfile.seek(0, os.SEEK_END)
            additional = b'\0' * ((64 * 1024 * 1024)-fvfile.tell())
            fvfile.write(additional)

        # QEMU must be on that path

        # Unique Command and Args parameters per ARCH
        if (self.env.GetValue("TARGET_ARCH").upper() == "AARCH64"):
            cmd = "qemu-system-aarch64"
            args = "-M virt"
            args += " -cpu cortex-a57"                                          # emulate cpu
        # elif(self.env.GetValue("TARGET_ARCH").upper() == "ARM"):
        #     cmd = "qemu-system-arm"
        #     args = "-M virt"
        #     args += " -cpu cortex-a15"                                          # emulate cpu
        else:
            raise NotImplementedError()

        # Common Args
        args += " -pflash " + Built_FV                                     # path to fw
        args += " -m 1024"                                                  # 1gb memory
        # turn off network
        args += " -net none"
        # Serial messages out
        args += " -serial stdio"
        # Mount disk with startup.nsh
        args += f" -drive file=fat:rw:{VirtualDrive},format=raw,media=disk"
        # Conditional Args
        if (self.env.GetValue("QEMU_HEADLESS").upper() == "TRUE"):
            args += " -display none"  # no graphics
        else:
            # args += " -device cirrus-vga"
            args += " -device virtio-gpu-pci"

        # if (self.env.GetValue("MAKE_STARTUP_NSH").upper() == "TRUE"):
        #     f = open(os.path.join(VirtualDrive, "startup.nsh"), "w")
        #     f.write("BOOT SUCCESS !!! \n")
        #     # add commands here
        #     f.write("reset -s\n")
        #     f.close()

        ret = RunCmd(cmd, args)

        if ret == 0xc0000005:
            # for some reason getting a c0000005 on successful return
            return 0

        return ret

        #Make virtual drive - Allow caller to override path otherwise use default
        # startup_nsh = StartUpScriptManager()
        # run_tests = (self.env.GetValue("RUN_TESTS", "FALSE").upper() == "TRUE")
        # output_base = self.env.GetValue("BUILD_OUTPUT_BASE")
        # shutdown_after_run = (self.env.GetValue("SHUTDOWN_AFTER_RUN", "FALSE").upper() == "TRUE")
        # empty_drive = (self.env.GetValue("EMPTY_DRIVE", "FALSE").upper() == "TRUE")

        # if os.name == 'nt':
            # VirtualDrivePath = self.env.GetValue("VIRTUAL_DRIVE_PATH", os.path.join(output_base, "VirtualDrive.vhd"))
            # VirtualDrive = VirtualDriveManager(VirtualDrivePath, self.env)
            # self.env.SetValue("VIRTUAL_DRIVE_PATH", VirtualDrivePath, "Set Virtual Drive path in case not set")
            # ut = UnitTestSupport(os.path.join(output_base, "X64"))

            # if empty_drive and os.path.isfile(VirtualDrivePath):
            #         os.remove(VirtualDrivePath)

            # if not os.path.isfile(VirtualDrivePath):
            #     VirtualDrive.MakeDrive()

            # test_regex = self.env.GetValue("TEST_REGEX", "")

            # if test_regex != "":
            #     ut.set_test_regex(test_regex)
            #     ut.find_tests()
            #     ut.copy_tests_to_virtual_drive(VirtualDrive)

            # if run_tests:
            #     if test_regex == "":
            #         logging.warning("No tests specified using TEST_REGEX flag but RUN_TESTS is TRUE")
            #     elif not empty_drive:
            #         logging.info("EMPTY_DRIVE=FALSE. This could impact your test results")

            #     if not shutdown_after_run:
            #         logging.info("SHUTDOWN_AFTER_RUN=FALSE (default). XML test results will not be \
            #             displayed until after the QEMU instance ends")
            #     ut.write_tests_to_startup_nsh(startup_nsh)

            # nshpath = os.path.join(output_base, "startup.nsh")
            # startup_nsh.WriteOut(nshpath, shutdown_after_run)

            # VirtualDrive.AddFile(nshpath)

        # else:
        #     VirtualDrivePath = self.env.GetValue("VIRTUAL_DRIVE_PATH", os.path.join(output_base, "VirtualDrive"))
        #     logging.warning("Linux currently isn't supported for the virtual drive. Falling back to an older method")

        #     if run_tests:
        #         logging.critical("Linux doesn't support running unit tests due to lack of VHD support")

        #     if os.path.exists(VirtualDrivePath) and empty_drive:
        #         shutil.rmtree(VirtualDrivePath)

        #     if not os.path.exists(VirtualDrivePath):
        #         os.makedirs(VirtualDrivePath)

        #     nshpath = os.path.join(VirtualDrivePath, "startup.nsh")
        #     self.env.SetValue("VIRTUAL_DRIVE_PATH", VirtualDrivePath, "Set Virtual Drive path in case not set")
        #     startup_nsh.WriteOut(nshpath, shutdown_after_run)

        # # Unique Command and Args parameters per ARCH
        # if (self.env.GetValue("TARGET_ARCH").upper() == "AARCH64"):
        #     cmd = "qemu-system-aarch64"
        #     args = "-M virt"
        #     args += " -cpu cortex-a57"                                          # emulate cpu
        # elif(self.env.GetValue("TARGET_ARCH").upper() == "ARM"):
        #     cmd = "qemu-system-arm"
        #     args = "-M virt"
        #     args += " -cpu cortex-a15"                                          # emulate cpu
        # else:
        #     raise NotImplementedError()

        # ret = self.Helper.QemuRun(self.env)
        # if ret != 0:
        #     logging.critical("Failed running Qemu")
        #     return ret

        # failures = 0
        # if run_tests and os.name == 'nt':
        #     failures = ut.report_results(VirtualDrive)

        # do stuff with unit test results here
        # return failures

# class UnitTestSupport(object):

#     def __init__(self, host_efi_build_output_path: os.PathLike):
#         self.test_list = []
#         self._globlist = []
#         self.host_efi_path = host_efi_build_output_path

#     def set_test_regex(self, csv_string):
#         self._globlist = csv_string.split(",")

#     def find_tests(self):
#         test_list = []
#         for globpattern in self._globlist:
#             test_list.extend(glob.glob(os.path.join(self.host_efi_path, globpattern)))
#         self.test_list = list(dict.fromkeys(test_list))

#     def copy_tests_to_virtual_drive(self, virtualdrive):
#         for test in self.test_list:
#             virtualdrive.AddFile(test)

#     def write_tests_to_startup_nsh(self,nshfile):
#         for test in self.test_list:
#             nshfile.AddLine(os.path.basename(test))

#     def report_results(self, virtualdrive) -> int:
#         from html import unescape

#         report_folder_path = os.path.join(os.path.dirname(virtualdrive.path_to_vhd), "unit_test_results")
#         os.makedirs(report_folder_path, exist_ok=True)
#         #now parse the xml for errors
#         failure_count = 0
#         logging.info("UnitTest Completed")
#         for unit_test in self.test_list:
#             xml_result_file = os.path.basename(unit_test)[:-4] + "_JUNIT.XML"
#             output_xml_file = os.path.join(report_folder_path, xml_result_file)
#             try:
#                 data = virtualdrive.GetFileContent(xml_result_file, output_xml_file)
#             except:
#                 logging.error(f"unit test ({unit_test}) produced no result file")
#                 failure_count += 1
#                 continue

#             logging.info('\n' + os.path.basename(unit_test) + "\n  Full Log: " + output_xml_file)

#             try:
#                 root = xml.etree.ElementTree.fromstring(data)
#                 for suite in root:
#                     logging.info(" ")
#                     for case in suite:
#                         logging.info('\t\t' + case.attrib['classname'] + " - ")
#                         caseresult = "\t\t\tPASS"
#                         level = logging.INFO
#                         for result in case:
#                             if result.tag == 'failure':
#                                 failure_count += 1
#                                 level = logging.ERROR
#                                 caseresult = "\t\tFAIL" + " - " + unescape(result.attrib['message'])
#                         logging.log( level, caseresult)
#             except Exception as ex:
#                 logging.error("Exception trying to read xml." + str(ex))
#                 failure_count += 1
#         return failure_count

# class VirtualDriveManager(object):

#     def __init__(self, vhd_path:os.PathLike, env:object):
#         self.path_to_vhd = os.path.abspath(vhd_path)
#         self._env = env

#     def MakeDrive(self, size: int=60):
#         ret = RunCmd("VHDCreate", f'-sz {size}MB {self.path_to_vhd}')
#         if ret != 0:
#             logging.error("Failed to create VHD")
#             return ret

#         ret = RunCmd("DiskFormat", f"-ft fat -ptt bios {self.path_to_vhd}")
#         if ret != 0:
#             logging.error("Failed to format VHD")
#             return ret
#         return ret

#     def AddFile(self, HostFilePath:os.PathLike):
#         file_name = os.path.basename(HostFilePath)
#         ret = RunCmd("FileInsert", f"{HostFilePath} {file_name} {self.path_to_vhd}")
#         return ret

#     def GetFileContent(self, VirtualFilePath, HostFilePath: os.PathLike=None):
#         temp_extract_path = HostFilePath
#         if temp_extract_path == None:
#             temp_extract_path = tempfile.mktemp()
#         logging.info(f"Extracting {VirtualFilePath} to {temp_extract_path}")
#         ret = self.ExtractFile(VirtualFilePath, temp_extract_path)
#         if ret != 0:
#             raise FileNotFoundError(VirtualFilePath)
#         with open(temp_extract_path, "rb") as f:
#             return f.read()

#     def ExtractFile(self, VirtualFilePath, HostFilePath:os.PathLike):
#         ret = RunCmd("FileExtract", f"{VirtualFilePath} {HostFilePath} {self.path_to_vhd}")
#         return ret


# class StartUpScriptManager(object):

#     FS_FINDER_SCRIPT = r'''
# #!/bin/nsh
# echo -off
# for %a run (0 10)
#     if exist fs%a:\{first_file} then
#         fs%a:
#         goto FOUND_IT
#     endif
# endfor

# :FOUND_IT
# '''

#     def __init__(self):
#         self._use_fs_finder = False
#         self._lines = []

#     def WriteOut(self, host_file_path, shutdown:bool):
#         with open(host_file_path, "w") as nsh:
#             if self._use_fs_finder:
#                 this_file = os.path.basename(host_file_path)
#                 nsh.write(StartUpScriptManager.FS_FINDER_SCRIPT.format(first_file=this_file))

#             for l in self._lines:
#                 nsh.write(l + "\n")

#             if shutdown:
#                 nsh.write("reset -s\n")

#     def AddLine(self, line):
#         self._lines.append(line.strip())
#         self._use_fs_finder = True

# if __name__ == "__main__":
#     import argparse
#     import sys
#     from edk2toolext.invocables.edk2_update import Edk2Update
#     from edk2toolext.invocables.edk2_setup import Edk2PlatformSetup
#     from edk2toolext.invocables.edk2_platform_build import Edk2PlatformBuild
#     print("Invoking Stuart")
#     print("     ) _     _")
#     print("    ( (^)-~-(^)")
#     print("__,-.\_( 0 0 )__,-.___")
#     print("  'W'   \   /   'W'")
#     print("         >o<")
#     SCRIPT_PATH = os.path.relpath(__file__)
#     parser = argparse.ArgumentParser(add_help=False)
#     parse_group = parser.add_mutually_exclusive_group()
#     parse_group.add_argument("--update", "--UPDATE",
#                              action='store_true', help="Invokes stuart_update")
#     parse_group.add_argument("--setup", "--SETUP",
#                              action='store_true', help="Invokes stuart_setup")
#     args, remaining = parser.parse_known_args()
#     new_args = ["stuart", "-c", SCRIPT_PATH]
#     new_args = new_args + remaining
#     sys.argv = new_args
#     if args.setup:
#         print("Running stuart_setup -c " + SCRIPT_PATH)
#         Edk2PlatformSetup().Invoke()
#     elif args.update:
#         print("Running stuart_update -c " + SCRIPT_PATH)
#         Edk2Update().Invoke()
#     else:
#         print("Running stuart_build -c " + SCRIPT_PATH)
#         Edk2PlatformBuild().Invoke()