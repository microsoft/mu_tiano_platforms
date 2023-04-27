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
import datetime
import xml.etree.ElementTree
import tempfile
import uuid
import string

from edk2toolext.environment import shell_environment
from edk2toolext.environment.uefi_build import UefiBuilder
from edk2toolext.invocables.edk2_platform_build import BuildSettingsManager
from edk2toolext.invocables.edk2_setup import SetupSettingsManager, RequiredSubmodule
from edk2toolext.invocables.edk2_update import UpdateSettingsManager
from edk2toolext.invocables.edk2_pr_eval import PrEvalSettingsManager
from edk2toollib.utility_functions import RunCmd, GetHostInfo
from typing import Tuple
from io import StringIO

# Declare test whose failure will not return a non-zero exit code
failure_exempt_tests = {}
failure_exempt_tests["BootAuditTestApp.efi"] = datetime.datetime(2023, 3, 7, 0, 0, 0)
failure_exempt_tests["LineParserTestApp.efi"] = datetime.datetime(2023, 3, 7, 0, 0, 0)
failure_exempt_tests["MorLockFunctionalTestApp.efi"] = datetime.datetime(2023, 3, 7, 0, 0, 0)
failure_exempt_tests["MsWheaEarlyUnitTestApp.efi"] = datetime.datetime(2023, 3, 7, 0, 0, 0)
failure_exempt_tests["VariablePolicyFuncTestApp.efi"] = datetime.datetime(2023, 3, 7, 0, 0, 0)
failure_exempt_tests["DxePagingAuditTestApp.efi"] = datetime.datetime(2023, 3, 7, 0, 0, 0)
failure_exempt_tests["MemoryProtectionTestApp.efi"] = datetime.datetime(2023, 4, 5, 0, 0, 0)
failure_exempt_tests["MemoryAttributeProtocolFuncTestApp.efi"] = datetime.datetime(2023, 4, 5, 0, 0, 0)

# Allow failure exempt tests to be ignored for 90 days
FAILURE_EXEMPT_OMISSION_LENGTH = 90*24*60*60

# Declare tests which require platform reset(s)
reset_tests = ["MorLockFunctionalTestApp.efi", "VariablePolicyFuncTestApp.efi"]

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
    Scopes = ('qemuq35', 'edk2-build', 'cibuild', 'configdata')
    WorkspaceRoot = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    PackagesPath = (
        "Platforms",
        "MU_BASECORE",
        "Common/MU",
        "Common/MU_TIANO",
        "Common/MU_OEM_SAMPLE",
        "Features/DFCI",
        "Features/CONFIG",
        "Features/MM_SUPV"
    )

    @staticmethod
    def add_common_command_line_options(parserObj) -> None:
        """Adds command line options common to settings managers."""
        parserObj.add_argument('--codeql', dest='codeql', action='store_true', default=False,
            help="Optional - Produces CodeQL results from the build. See "
                 "MU_BASECORE/.pytool/Plugin/CodeQL/Readme.md for more information.")

    @staticmethod
    def retrieve_common_command_line_options(args) -> bool:
        """Retrieves command line options common to settings managers."""
        return args.codeql

    @staticmethod
    def get_active_scopes(codeql_enabled: bool) -> Tuple[str]:
        """Returns the active scopes for the platform."""
        active_scopes = CommonPlatform.Scopes

        # Enable the CodeQL plugin if chosen on command line
        if codeql_enabled:
            if GetHostInfo().os == "Linux":
                active_scopes += ("codeql-linux-ext-dep",)
            else:
                active_scopes += ("codeql-windows-ext-dep",)
            active_scopes += ("codeql-build", "codeql-analyze")

        return active_scopes

    # ####################################################################################### #
    #                         Configuration for Update & Setup                                #
    # ####################################################################################### #
class SettingsManager(UpdateSettingsManager, SetupSettingsManager, PrEvalSettingsManager):

    def AddCommandLineOptions(self, parserObj):
        """Add command line options to the argparser"""
        CommonPlatform.add_common_command_line_options(parserObj)

    def RetrieveCommandLineOptions(self, args):
        """Retrieve command line options from the argparser"""
        self.codeql = CommonPlatform.retrieve_common_command_line_options(args)

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
        """Return iterable containing RequiredSubmodule objects.

        !!! note
            If no RequiredSubmodules return an empty iterable
        """
        return [
            RequiredSubmodule("MU_BASECORE", True),
            RequiredSubmodule("Common/MU", True),
            RequiredSubmodule("Common/MU_TIANO", True),
            RequiredSubmodule("Common/MU_OEM_SAMPLE", True),
            RequiredSubmodule("Features/DFCI", True),
            RequiredSubmodule("Features/CONFIG", True),
            RequiredSubmodule("Features/MM_SUPV", True),
        ]

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
        return CommonPlatform.get_active_scopes(self.codeql)

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
class PlatformBuilder(UefiBuilder, BuildSettingsManager):
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

        CommonPlatform.add_common_command_line_options(parserObj)

    def RetrieveCommandLineOptions(self, args):
        '''  Retrieve command line options from the argparser '''
        if args.build_arch.upper() != "IA32,X64":
            raise Exception("Invalid Arch Specified.  Please see comments in PlatformBuild.py::PlatformBuilder::AddCommandLineOptions")

        self.codeql = CommonPlatform.retrieve_common_command_line_options(args)

    def GetWorkspaceRoot(self):
        ''' get WorkspacePath '''
        return CommonPlatform.WorkspaceRoot

    def GetPackagesPath(self):
        ''' Return a list of workspace relative paths that should be mapped as edk2 PackagesPath '''
        result = [
            shell_environment.GetBuildVars().GetValue("FEATURE_CONFIG_PATH", ""),
            shell_environment.GetBuildVars().GetValue("FEATURE_MM_SUPV_PATH", "")
        ]
        for a in CommonPlatform.PackagesPath:
            result.append(a)
        return result

    def GetActiveScopes(self):
        ''' return tuple containing scopes that should be active for this process '''
        return CommonPlatform.get_active_scopes(self.codeql)

    def GetName(self):
        ''' Get the name of the repo, platform, or product being build '''
        ''' Used for naming the log file, among others '''
        # Check the FlashImage bool and rename the log file if true.
        # Two builds are done during CI: one without the --FlashOnly flag
        # followed by one with the flag. self.FlashImage will be true if the
        # --FlashOnly flag is passed, meaning we will keep separate build and run logs
        if(self.FlashImage):
            return "QemuQ35Pkg_Run"
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
        self.env.SetValue("EMPTY_DRIVE", "FALSE", "Default to false")
        self.env.SetValue("RUN_TESTS", "FALSE", "Default to false")
        self.env.SetValue("QEMU_HEADLESS", "FALSE", "Default to false")
        self.env.SetValue("SHUTDOWN_AFTER_RUN", "FALSE", "Default to false")
        # needed to make FV size build report happy
        self.env.SetValue("BLD_*_BUILDID_STRING", "Unknown", "Default")
        # Default turn on build reporting.
        self.env.SetValue("BUILDREPORTING", "TRUE", "Enabling build report")
        self.env.SetValue("BUILDREPORT_TYPES", "PCD DEPEX FLASH BUILD_FLAGS LIBRARY FIXED_ADDRESS HASH", "Setting build report types")
        self.env.SetValue("BLD_*_QEMU_CORE_NUM", "4", "Default")
        self.env.SetValue("BLD_*_MEMORY_PROTECTION", "TRUE", "Default")
        # Include the MFCI test cert by default, override on the commandline with "BLD_*_SHIP_MODE=TRUE" if you want the retail MFCI cert
        self.env.SetValue("BLD_*_SHIP_MODE", "FALSE", "Default")
        self.env.SetValue("CONF_AUTOGEN_INCLUDE_PATH", self.mws.join(self.ws, "Platforms", "QemuQ35Pkg", "Include"), "Platform Defined")

        self.env.SetValue("YAML_POLICY_FILE", self.mws.join(self.ws, "QemuQ35Pkg", "PolicyData", "PolicyDataUsb.yaml"), "Platform Hardcoded")
        self.env.SetValue("POLICY_DATA_STRUCT_FOLDER", self.mws.join(self.ws, "QemuQ35Pkg", "Include"), "Platform Defined")
        self.env.SetValue('POLICY_REPORT_FOLDER', self.mws.join(self.ws, "QemuQ35Pkg", "PolicyData"), "Platform Defined")
        self.env.SetValue('MU_SCHEMA_DIR', self.mws.join(self.ws, "Platforms", "QemuQ35Pkg", "CfgData"), "Platform Defined")
        self.env.SetValue('MU_SCHEMA_FILE_NAME', "QemuQ35PkgCfgData.xml", "Platform Hardcoded")

        # Globally set CodeQL failures to be ignored in this repo.
        # Note: This has no impact if CodeQL is not active/enabled.
        self.env.SetValue("STUART_CODEQL_AUDIT_ONLY", "true", "Platform Defined")

        # Enabled all of the SMM modules
        self.env.SetValue("BLD_*_SMM_ENABLED", "TRUE", "Default")

        return 0

    def PlatformPreBuild(self):
        # Here we build the secure policy blob for build system to use and add into the targeted FV
        policy_example_dir = self.mws.join(self.mws.WORKSPACE, "MmSupervisorPkg", "SupervisorPolicyTools", "MmIsolationPoliciesExample.xml")
        output_dir = os.path.join(self.env.GetValue("BUILD_OUTPUT_BASE"), "Policy")
        if (not os.path.isdir(output_dir)):
            os.makedirs (output_dir)
        output_name = os.path.join(output_dir, "secure_policy.bin")

        ret = self.Helper.MakeSupervisorPolicy(xml_file_path=policy_example_dir, output_binary_path=output_name)
        if(ret != 0):
            raise Exception("SupervisorPolicyMaker Failed: Errorcode %d" % ret)
        self.env.SetValue("BLD_*_POLICY_BIN_PATH", output_name, "Set generated secure policy path")
        return ret

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
        test_regex = self.env.GetValue("TEST_REGEX", "")

        if os.name == 'nt':
            VirtualDrivePath = self.env.GetValue("VIRTUAL_DRIVE_PATH", os.path.join(output_base, "VirtualDrive.vhd"))
            VirtualDrive = WindowsVirtualDriveManager(VirtualDrivePath, self.env)
        else:
            VirtualDrivePath = self.env.GetValue("VIRTUAL_DRIVE_PATH", os.path.join(output_base, "VirtualDrive.img"))
            VirtualDrive = LinuxVirtualDriveManager (VirtualDrivePath)

        self.env.SetValue("VIRTUAL_DRIVE_PATH", VirtualDrivePath, "Set Virtual Drive path in case not set")

        if empty_drive and os.path.isfile(VirtualDrivePath):
            os.remove(VirtualDrivePath)

        if not os.path.isfile(VirtualDrivePath):
            VirtualDrive.MakeDrive()

        ut = UnitTestSupport(os.path.join(output_base, "X64"))
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

        # Get the version number (repo release)
        outstream = StringIO()
        version = "Unknown"
        ret = RunCmd('git', "rev-parse HEAD", outstream=outstream)
        if ret == 0:
            commithash = outstream.getvalue().strip()
            outstream = StringIO()
            # See git-describe docs for a breakdown of this command output
            ret = RunCmd("git", f'describe {commithash} --tags', outstream=outstream)
            if ret == 0:
                version = outstream.getvalue().strip()

        self.env.SetValue("VERSION", version, "Set Version value")

        ret = self.Helper.QemuRun(self.env)
        if ret != 0:
            logging.critical("Failed running Qemu")
            return ret

        failures = 0
        if run_tests:
            failures = ut.report_results(VirtualDrive, self.env)

        # do stuff with unit test results here
        return failures

class UnitTestSupport(object):
    paging_audit_data_files = ["1G.dat", "2M.dat", "4K.dat", "PDE.dat", "MAT.dat", 
                               "GuardPage.dat", "MemoryInfoDatabase.dat"]
    paging_audit_generator_path = os.path.join("Common", "MU", "UefiTestingPkg", "AuditTests", 
                                               "PagingAudit", "Windows", "PagingReportGenerator.py")

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
            if (os.path.basename(test) in reset_tests):
                nshfile.AddLine(os.path.basename(test))
        for test in self.test_list:
            if not (os.path.basename(test) in reset_tests):
                nshfile.AddLine(os.path.basename(test))
                # Also run DxePagingAuditTestApp.efi with the -d option
                if ("DxePagingAuditTestApp" in os.path.basename(test)):
                    nshfile.AddLine(f"{os.path.basename(test)} -d")
    
    def generate_paging_audit(self, virtualdrive, report_folder_path, env):
        version = env.GetValue("VERSION", "Unknown")

        for file in self.paging_audit_data_files:
            virtualdrive.ExtractFile(file, os.path.join(report_folder_path, file))
        output_audit = os.path.join(report_folder_path, "pagingaudit.html")
        output_debug = os.path.join(report_folder_path, "pagingauditdebug.txt")
        RunCmd("python", f"{self.paging_audit_generator_path} -i {report_folder_path} \
-o {output_audit} -p Q35 -t DXE --debug -l {output_debug} -a X64 --PlatformVersion {version}")

    def report_results(self, virtualdrive, env) -> int:
        from html import unescape

        report_folder_path = os.path.join(os.path.dirname(virtualdrive.drive_path), "unit_test_results")
        os.makedirs(report_folder_path, exist_ok=True)
        #now parse the xml for errors
        failure_count = 0
        logging.info("UnitTest(s) Completed")
        for unit_test in self.test_list:

            # If the test is DxePagingAuditTestApp.efi, run the paging audit generator
            if (os.path.basename(unit_test) == "DxePagingAuditTestApp.efi"):
                self.generate_paging_audit(virtualdrive, report_folder_path, env)

            ignore_failure = False
            if (os.path.basename(unit_test) in failure_exempt_tests.keys()):
                now = datetime.datetime.now()
                last_ignore_time = failure_exempt_tests[os.path.basename(unit_test)]
                if (now - last_ignore_time).total_seconds() < FAILURE_EXEMPT_OMISSION_LENGTH:
                    logging.info("Ignoring output of " + os.path.basename(unit_test))
                    ignore_failure = True
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
                                level = logging.ERROR
                                caseresult = "\t\tFAIL" + " - " + unescape(result.attrib['message'])
                                if not ignore_failure:
                                    failure_count += 1
                        logging.log( level, caseresult)
            except Exception as ex:
                logging.error("Exception trying to read xml." + str(ex))
                failure_count += 1
        return failure_count



class WindowsVirtualDriveManager(object):

    def __init__(self, vhd_path:os.PathLike, env:object):
        self.drive_path = os.path.abspath(vhd_path)
        self._env = env

    def MakeDrive(self, size: int=60):
        ret = RunCmd("VHDCreate", f'-sz {size}MB {self.drive_path}')
        if ret != 0:
            logging.error("Failed to create VHD")
            return ret

        ret = RunCmd("DiskFormat", f"-ft fat -ptt bios {self.drive_path}")
        if ret != 0:
            logging.error("Failed to format VHD")
            return ret
        return ret

    def AddFile(self, HostFilePath:os.PathLike):
        file_name = os.path.basename(HostFilePath)
        ret = RunCmd("FileInsert", f"{HostFilePath} {file_name} {self.drive_path}")
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
        ret = RunCmd("FileExtract", f"{VirtualFilePath} {HostFilePath} {self.drive_path}")
        return ret

class LinuxVirtualDriveManager(object):

    def __init__(self, img_path:os.PathLike):
        self.drive_path = os.path.abspath(img_path)
        self.drive_letter = self.find_unused_drive_letter()

    def find_unused_drive_letter(self):
        for drive_letter in string.ascii_lowercase:
            mtab_content = os.popen(f"grep -i '/mnt/{drive_letter} ' /etc/mtab").read()
            if mtab_content:
                continue
            return drive_letter

        raise ValueError("No unused drive letters available")

    def MakeDrive(self, size: int=60):
        # Create an image
        ret = RunCmd("dd", f"if=/dev/zero of={self.drive_path} bs=1M count={size}")
        if ret != 0:
            logging.error("Failed to create IMG")
            return ret

        # Format the image as FAT32
        ret = RunCmd("mkfs.vfat", f"{self.drive_path}")
        if ret != 0:
            logging.error("Failed to format IMG")
            return ret

        # Create an mtools config file to virtually map the image to a drive letter
        RunCmd("echo", "mtools_skip_check=1 > ~/.mtoolsrc")
        RunCmd("echo", f"drive {self.drive_letter}: >> ~/.mtoolsrc")
        RunCmd("echo", f"\"  file=\\\"{self.drive_path}\\\" exclusive\" >> ~/.mtoolsrc")

        return 0

    def AddFile(self, HostFilePath:os.PathLike):
        ret = RunCmd("mcopy", f"-n -i {self.drive_path} {HostFilePath} {self.drive_letter}:")
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
        full_path = os.path.join(self.drive_letter + ":", VirtualFilePath)
        ret = RunCmd("mcopy", f"-n -i {self.drive_path} {full_path} {HostFilePath}")
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