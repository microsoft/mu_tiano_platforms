# @file
# Script to Build QEMU SBSA Platform UEFI firmware
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
import uuid

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
    PackagesSupported = ("QemuSbsaPkg",)
    ArchSupported = ("AARCH64",)
    TargetsSupported = ("DEBUG", "RELEASE", "NOOPT")
    Scopes = ('qemusbsa', 'gcc_aarch64_linux', 'edk2-build', 'cibuild', 'setupdata')
    WorkspaceRoot = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    PackagesPath = ("Platforms", "MU_BASECORE", "Common/MU", "Common/MU_TIANO", "Common/MU_OEM_SAMPLE", "Silicon/Arm/MU_TIANO", "Silicon/Arm/TFA")


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
        return ("QemuSbsaPkg/QemuSbsaPkg.dsc", {})

    def GetName(self):
        return "QemuSbsa"

    def GetPackagesPath(self):
        ''' Return a list of paths that should be mapped as edk2 PackagesPath '''
        result = [
            shell_environment.GetBuildVars().GetValue("FEATURE_CONFIG_PATH", "")
        ]
        for a in CommonPlatform.PackagesPath:
            result.append(a)
        return result

    # ####################################################################################### #
    #                         Actual Configuration for Platform Build                         #
    # ####################################################################################### #
class PlatformBuilder( UefiBuilder, BuildSettingsManager):
    def __init__(self):
        UefiBuilder.__init__(self)

    def CleanTree(self, RemoveConfTemplateFilesToo=False):
        # Add a step to clean up BL31 as well, if asked
        cmd = "make"
        args = "distclean"
        ret = RunCmd(cmd, args, workingdir= self.env.GetValue("ARM_TFA_PATH"))
        if ret != 0:
            return ret

        return super().CleanTree(RemoveConfTemplateFilesToo)

    def AddCommandLineOptions(self, parserObj):
        ''' Add command line options to the argparser '''

        # In an effort to support common server based builds this parameter is added.  It is
        # checked for correctness but is never uses as this platform only supports a single set of
        # architectures.
        parserObj.add_argument('-a', "--arch", dest="build_arch", type=str, default="AARCH64",
            help="Optional - CSV of architecture to build.  AARCH64 is used for PEI and "
            "DXE and is the only valid option for this platform.")

    def RetrieveCommandLineOptions(self, args):
        '''  Retrieve command line options from the argparser '''
        if args.build_arch.upper() != "AARCH64":
            raise Exception("Invalid Arch Specified.  Please see comments in PlatformBuild.py::PlatformBuilder::AddCommandLineOptions")

        shell_environment.GetBuildVars().SetValue(
            "TARGET_ARCH", args.build_arch.upper(), "From CmdLine")

        shell_environment.GetBuildVars().SetValue(
            "ACTIVE_PLATFORM", "QemuSbsaPkg/QemuSbsaPkg.dsc", "From CmdLine")


    def GetWorkspaceRoot(self):
        ''' get WorkspacePath '''
        return CommonPlatform.WorkspaceRoot

    def GetPackagesPath(self):
        ''' Return a list of paths that should be mapped as edk2 PackagesPath '''
        result = [
            shell_environment.GetBuildVars().GetValue("FEATURE_CONFIG_PATH", "")
        ]
        for a in CommonPlatform.PackagesPath:
            result.append(a)
        return result

    def GetActiveScopes(self):
        ''' return tuple containing scopes that should be active for this process '''
        return CommonPlatform.Scopes

    def GetName(self):
        ''' Get the name of the repo, platform, or product being build '''
        ''' Used for naming the log file, among others '''
        # Check the FlashImage bool and rename the log file if true.
        # Two builds are done during CI: one without the --FlashOnly flag
        # followed by one with the flag. self.FlashImage will be true if the
        # --FlashOnly flag is passed, meaning we will keep separate build and run logs
        if(self.FlashImage):
            return "QemuSbsaPkg_Run"
        return "QemuSbsaPkg"

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
        self.env.SetValue("PRODUCT_NAME", "QemuSbsa", "Platform Hardcoded")
        self.env.SetValue("ACTIVE_PLATFORM", "QemuSbsaPkg/QemuSbsaPkg.dsc", "Platform Hardcoded")
        self.env.SetValue("TARGET_ARCH", "AARCH64", "Platform Hardcoded")
        self.env.SetValue("TOOL_CHAIN_TAG", "GCC5", "set default to gcc5")
        # self.env.SetValue("EMPTY_DRIVE", "FALSE", "Default to false")
        # self.env.SetValue("RUN_TESTS", "FALSE", "Default to false")
        self.env.SetValue("QEMU_HEADLESS", "FALSE", "Default to false")
        self.env.SetValue("QEMU_PLATFORM", "qemu_sbsa", "Platform Hardcoded")
        # self.env.SetValue("SHUTDOWN_AFTER_RUN", "FALSE", "Default to false")
        # needed to make FV size build report happy
        # self.env.SetValue("BLD_*_BUILDID_STRING", "Unknown", "Default")
        # # Default turn on build reporting.
        self.env.SetValue("BUILDREPORTING", "TRUE", "Enabling build report")
        self.env.SetValue("BUILDREPORT_TYPES", "PCD DEPEX FLASH BUILD_FLAGS LIBRARY FIXED_ADDRESS HASH", "Setting build report types")
        self.env.SetValue("ARM_TFA_PATH", os.path.join (self.GetWorkspaceRoot (), "Silicon/Arm/TFA"), "Platform hardcoded")
        self.env.SetValue("BLD_*_QEMU_CORE_NUM", "4", "Default")
        # Include the MFCI test cert by default, override on the commandline with "BLD_*_SHIP_MODE=TRUE" if you want the retail MFCI cert
        self.env.SetValue("BLD_*_SHIP_MODE", "FALSE", "Default")
        self.__SetEsrtGuidVars("CONF_POLICY_GUID", "6E08E434-8E04-47B5-9A77-78A3A24523EA", "Platform Hardcoded")
        self.env.SetValue("YAML_CONF_FILE", self.mws.join(self.ws, "QemuQ35Pkg", "CfgData", "CfgDataDef.yaml"), "Platform Hardcoded")
        self.env.SetValue("DELTA_CONF_POLICY", self.mws.join(self.ws, "QemuQ35Pkg", "CfgData", "Profile1.dlt") + ";" +\
                          self.mws.join(self.ws, "QemuQ35Pkg", "CfgData", "Profile2.dlt") + ";" +\
                          self.mws.join(self.ws, "QemuQ35Pkg", "CfgData", "Profile3.dlt"), "Platform Hardcoded")
        self.env.SetValue("CONF_DATA_STRUCT_FOLDER", self.mws.join(self.ws, "QemuQ35Pkg", "Include"), "Platform Defined")
        self.env.SetValue('CONF_REPORT_FOLDER', self.mws.join(self.ws, "QemuQ35Pkg", "CfgData"), "Platform Defined")

        self.env.SetValue("YAML_POLICY_FILE", self.mws.join(self.ws, "QemuQ35Pkg", "PolicyData", "PolicyDataUsb.yaml"), "Platform Hardcoded")
        self.env.SetValue("POLICY_DATA_STRUCT_FOLDER", self.mws.join(self.ws, "QemuQ35Pkg", "Include"), "Platform Defined")
        self.env.SetValue('POLICY_REPORT_FOLDER', self.mws.join(self.ws, "QemuQ35Pkg", "PolicyData"), "Platform Defined")

        return 0

    def PlatformPreBuild(self):
        return 0

    #
    # Copy a file into the designated region of target FD.
    #
    def PatchRegion(self, fdfile, mainStart, size, srcfile):
        with open(fdfile, "r+b") as fd, open(srcfile, "rb") as src:
            fd.seek(mainStart)
            patchImage = src.read(size)
            fd.seek(mainStart)
            fd.write(patchImage)
        return 0

    def PlatformPostBuild(self):
        # Add a post build step to build BL31 and assemble the FD files
        op_fv = os.path.join(self.env.GetValue("BUILD_OUTPUT_BASE"), "FV")

        logging.info("Building TF-A")
        cmd = "make"
        args = "CROSS_COMPILE=" + shell_environment.GetEnvironment().get_shell_var("GCC5_AARCH64_PREFIX")
        args += " PLAT=" + self.env.GetValue("QEMU_PLATFORM").lower()
        args += " ARCH=" + self.env.GetValue("TARGET_ARCH").lower()
        args += " DEBUG=" + str(1 if self.env.GetValue("TARGET").lower() == 'debug' else 0)
        args += " SPM_MM=1 EL3_EXCEPTION_HANDLING=1"
        args += " BL32=" + os.path.join(op_fv, "BL32_AP_MM.fd")
        args += " all fip"
        args += " -j $(nproc)"
        ret = RunCmd(cmd, args, workingdir= self.env.GetValue("ARM_TFA_PATH"))
        if ret != 0:
            return ret

        # Now that BL31 is built with BL32 supplied, patch BL1 and BL31 built fip.bin into the SECURE_FLASH0.fd
        op_tfa = os.path.join (
            self.env.GetValue("ARM_TFA_PATH"), "build",
            self.env.GetValue("QEMU_PLATFORM").lower(),
            self.env.GetValue("TARGET").lower())

        logging.info("Patching BL1 region")
        print (self.env.GetValue("SECURE_FLASH_REGION_BL1_OFFSET"))
        self.PatchRegion(
            os.path.join(op_fv, "SECURE_FLASH0.fd"),
            int(self.env.GetValue("SECURE_FLASH_REGION_BL1_OFFSET"), 16),
            int( self.env.GetValue("SECURE_FLASH_REGION_BL1_SIZE"), 16),
            os.path.join(op_tfa, "bl1.bin"),
            )

        logging.info("Patching FIP region")
        self.PatchRegion(
            os.path.join(op_fv, "SECURE_FLASH0.fd"),
            int(self.env.GetValue("SECURE_FLASH_REGION_FIP_OFFSET"), 16),
            int( self.env.GetValue("SECURE_FLASH_REGION_FIP_SIZE"), 16),
            os.path.join(op_tfa, "fip.bin")
            )

        # Pad both fd to 256mb, as required by QEMU
        OutputPath_FV = os.path.join(self.env.GetValue("BUILD_OUTPUT_BASE"), "FV")
        Built_FV = os.path.join(OutputPath_FV, "QEMU_EFI.fd")
        with open(Built_FV, "ab") as fvfile:
            fvfile.seek(0, os.SEEK_END)
            additional = b'\0' * ((256 * 1024 * 1024)-fvfile.tell())
            fvfile.write(additional)

        bl3 = os.path.join(OutputPath_FV, "SECURE_FLASH0.fd")
        with open(bl3, "ab") as fvfile:
            fvfile.seek(0, os.SEEK_END)
            additional = b'\0' * ((256 * 1024 * 1024)-fvfile.tell())
            fvfile.write(additional)

        return 0

    def __SetEsrtGuidVars(self, var_name, guid_str, desc_string):
        cur_guid = uuid.UUID(guid_str)
        self.env.SetValue("BLD_*_%s_REGISTRY" % var_name, guid_str, desc_string)
        self.env.SetValue("BLD_*_%s_BYTES" % var_name, "'{" + (",".join(("0x%X" % byte) for byte in cur_guid.bytes_le)) + "}'", desc_string)
        return

    def FlashRomImage(self):
        #Make virtual drive - Allow caller to override path otherwise use default
        startup_nsh = StartUpScriptManager()
        run_tests = (self.env.GetValue("RUN_TESTS", "FALSE").upper() == "TRUE")
        output_base = self.env.GetValue("BUILD_OUTPUT_BASE")
        shutdown_after_run = (self.env.GetValue("SHUTDOWN_AFTER_RUN", "FALSE").upper() == "TRUE")
        empty_drive = (self.env.GetValue("EMPTY_DRIVE", "FALSE").upper() == "TRUE")

        if os.name == 'nt':
            VirtualDrivePath = self.env.GetValue("VIRTUAL_DRIVE_PATH", os.path.join(output_base, "VirtualDrive.vhd"))
            VirtualDrive = VirtualDriveManager(VirtualDrivePath, self.env)
            self.env.SetValue("VIRTUAL_DRIVE_PATH", VirtualDrivePath, "Set Virtual Drive path in case not set")
            ut = UnitTestSupport(os.path.join(output_base, "AARCH64"))

            if empty_drive and os.path.isfile(VirtualDrivePath):
                    os.remove(VirtualDrivePath)

            if not os.path.isfile(VirtualDrivePath):
                VirtualDrive.MakeDrive()

            test_regex = self.env.GetValue("TEST_REGEX", "")

            if test_regex != "":
                ut.set_test_regex(test_regex)
                ut.find_tests()
                ut.copy_tests_to_virtual_drive(VirtualDrive)

            if run_tests:
                if test_regex == "":
                    logging.warning("No tests specified using TEST_REGEX flag but RUN_TESTS is TRUE")
                elif not empty_drive:
                    logging.info("EMPTY_DRIVE=FALSE. This could impact your test results")

                if not shutdown_after_run:
                    logging.info("SHUTDOWN_AFTER_RUN=FALSE (default). XML test results will not be \
                        displayed until after the QEMU instance ends")
                ut.write_tests_to_startup_nsh(startup_nsh)

            nshpath = os.path.join(output_base, "startup.nsh")
            startup_nsh.WriteOut(nshpath, shutdown_after_run)

            VirtualDrive.AddFile(nshpath)

        else:
            VirtualDrivePath = self.env.GetValue("VIRTUAL_DRIVE_PATH", os.path.join(output_base, "VirtualDrive"))
            logging.warning("Linux currently isn't supported for the virtual drive. Falling back to an older method")

            if run_tests:
                logging.critical("Linux doesn't support running unit tests due to lack of VHD support")

            if os.path.exists(VirtualDrivePath) and empty_drive:
                shutil.rmtree(VirtualDrivePath)

            if not os.path.exists(VirtualDrivePath):
                os.makedirs(VirtualDrivePath)

            nshpath = os.path.join(VirtualDrivePath, "startup.nsh")
            self.env.SetValue("VIRTUAL_DRIVE_PATH", VirtualDrivePath, "Set Virtual Drive path in case not set")
            startup_nsh.WriteOut(nshpath, shutdown_after_run)

        ret = self.Helper.QemuRun(self.env)
        if ret != 0:
            logging.critical("Failed running Qemu")
            return ret

        failures = 0
        if run_tests and os.name == 'nt':
            failures = ut.report_results(VirtualDrive)

        # do stuff with unit test results here
        return failures

class UnitTestSupport(object):

    def __init__(self, host_efi_build_output_path: os.PathLike):
        self.test_list = []
        self._globlist = []
        self.host_efi_path = host_efi_build_output_path

    def set_test_regex(self, csv_string):
        self._globlist = csv_string.split(",")

    def find_tests(self):
        test_list = []
        for globpattern in self._globlist:
            test_list.extend(glob.glob(os.path.join(self.host_efi_path, globpattern)))
        self.test_list = list(dict.fromkeys(test_list))

    def copy_tests_to_virtual_drive(self, virtualdrive):
        for test in self.test_list:
            virtualdrive.AddFile(test)

    def write_tests_to_startup_nsh(self,nshfile):
        for test in self.test_list:
            nshfile.AddLine(os.path.basename(test))

    def report_results(self, virtualdrive) -> int:
        from html import unescape

        report_folder_path = os.path.join(os.path.dirname(virtualdrive.path_to_vhd), "unit_test_results")
        os.makedirs(report_folder_path, exist_ok=True)
        #now parse the xml for errors
        failure_count = 0
        logging.info("UnitTest Completed")
        for unit_test in self.test_list:
            xml_result_file = os.path.basename(unit_test)[:-4] + "_JUNIT.XML"
            output_xml_file = os.path.join(report_folder_path, xml_result_file)
            try:
                data = virtualdrive.GetFileContent(xml_result_file, output_xml_file)
            except:
                logging.error(f"unit test ({unit_test}) produced no result file")
                failure_count += 1
                continue

            logging.info('\n' + os.path.basename(unit_test) + "\n  Full Log: " + output_xml_file)

            try:
                root = xml.etree.ElementTree.fromstring(data)
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
                                caseresult = "\t\tFAIL" + " - " + unescape(result.attrib['message'])
                        logging.log( level, caseresult)
            except Exception as ex:
                logging.error("Exception trying to read xml." + str(ex))
                failure_count += 1
        return failure_count

class VirtualDriveManager(object):

    def __init__(self, vhd_path:os.PathLike, env:object):
        self.path_to_vhd = os.path.abspath(vhd_path)
        self._env = env

    def MakeDrive(self, size: int=60):
        ret = RunCmd("VHDCreate", f'-sz {size}MB {self.path_to_vhd}')
        if ret != 0:
            logging.error("Failed to create VHD")
            return ret

        ret = RunCmd("DiskFormat", f"-ft fat -ptt bios {self.path_to_vhd}")
        if ret != 0:
            logging.error("Failed to format VHD")
            return ret
        return ret

    def AddFile(self, HostFilePath:os.PathLike):
        file_name = os.path.basename(HostFilePath)
        ret = RunCmd("FileInsert", f"{HostFilePath} {file_name} {self.path_to_vhd}")
        return ret

    def GetFileContent(self, VirtualFilePath, HostFilePath: os.PathLike=None):
        temp_extract_path = HostFilePath
        if temp_extract_path == None:
            temp_extract_path = tempfile.mktemp()
        logging.info(f"Extracting {VirtualFilePath} to {temp_extract_path}")
        ret = self.ExtractFile(VirtualFilePath, temp_extract_path)
        if ret != 0:
            raise FileNotFoundError(VirtualFilePath)
        with open(temp_extract_path, "rb") as f:
            return f.read()

    def ExtractFile(self, VirtualFilePath, HostFilePath:os.PathLike):
        ret = RunCmd("FileExtract", f"{VirtualFilePath} {HostFilePath} {self.path_to_vhd}")
        return ret


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

    def WriteOut(self, host_file_path, shutdown:bool):
        with open(host_file_path, "w") as nsh:
            if self._use_fs_finder:
                this_file = os.path.basename(host_file_path)
                nsh.write(StartUpScriptManager.FS_FINDER_SCRIPT.format(first_file=this_file))

            for l in self._lines:
                nsh.write(l + "\n")

            if shutdown:
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