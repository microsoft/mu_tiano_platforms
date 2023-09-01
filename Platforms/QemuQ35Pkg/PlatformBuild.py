# @file
# Script to Build QemuQ35 Mu UEFI firmware
#
# Copyright (c) Microsoft Corporation.
# SPDX-License-Identifier: BSD-2-Clause-Patent
##
import os
import logging
import glob
import datetime
import sys
import uuid

from edk2toolext import codeql as codeql_helpers
from edk2toolext.environment import shell_environment
from edk2toolext.environment.uefi_build import UefiBuilder
from edk2toolext.invocables.edk2_platform_build import BuildSettingsManager
from edk2toolext.invocables.edk2_setup import SetupSettingsManager, RequiredSubmodule
from edk2toolext.invocables.edk2_update import UpdateSettingsManager
from edk2toolext.invocables.edk2_pr_eval import PrEvalSettingsManager
from edk2toollib.utility_functions import RunCmd, GetHostInfo
from typing import Tuple
from pathlib import Path
from io import StringIO

WORKSPACE_ROOT = Path(__file__).parent.parent.parent.cwd()

# Declare test whose failure will not return a non-zero exit code
FAILURE_EXEMPT_TESTS = {
    "VariablePolicyFuncTestApp.efi": datetime.datetime(2023, 7, 20, 0, 0, 0),
}

# Allow failure exempt tests to be ignored for 90 days
FAILURE_EXEMPT_OMISSION_LENGTH = 90*24*60*60


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
    Scopes = ('qemu', 'qemuq35', 'edk2-build', 'cibuild', 'configdata')
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
        codeql_helpers.add_command_line_option(parserObj)

    @staticmethod
    def is_codeql_enabled(args) -> bool:
        """Retrieves whether CodeQL is enabled."""
        return codeql_helpers.is_codeql_enabled_on_command_line(args)

    @staticmethod
    def get_active_scopes(codeql_enabled: bool) -> Tuple[str]:
        """Returns the active scopes for the platform."""
        active_scopes = CommonPlatform.Scopes
        active_scopes += codeql_helpers.get_scopes(codeql_enabled)

        if codeql_enabled:
            codeql_filter_files = [str(n) for n in glob.glob(
                os.path.join(WORKSPACE_ROOT,
                                '**/CodeQlFilters.yml'), recursive=True)]
            shell_environment.GetBuildVars().SetValue(
                "STUART_CODEQL_FILTER_FILES",
                ','.join(codeql_filter_files),
                "Set in CISettings.py")

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
        self.codeql = CommonPlatform.is_codeql_enabled(args)

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
        return str(WORKSPACE_ROOT)

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

        self.codeql = CommonPlatform.is_codeql_enabled(args)

    def GetWorkspaceRoot(self):
        ''' get WorkspacePath '''
        return str(WORKSPACE_ROOT)

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
        self.env.SetValue("CONF_AUTOGEN_INCLUDE_PATH", self.edk2path.GetAbsolutePathOnThisSystemFromEdk2RelativePath("QemuQ35Pkg", "Include"), "Platform Defined")

        self.env.SetValue('MU_SCHEMA_DIR', self.edk2path.GetAbsolutePathOnThisSystemFromEdk2RelativePath("QemuQ35Pkg", "CfgData"), "Platform Defined")
        self.env.SetValue('MU_SCHEMA_FILE_NAME', "QemuQ35PkgCfgData.xml", "Platform Hardcoded")
        self.env.SetValue('CONF_PROFILE_PATHS',
                          self.edk2path.GetAbsolutePathOnThisSystemFromEdk2RelativePath('QemuQ35Pkg', 'CfgData', 'Profile0QemuQ35PkgCfgData.csv') + " " +
                          self.edk2path.GetAbsolutePathOnThisSystemFromEdk2RelativePath('QemuQ35Pkg', 'CfgData', 'Profile1QemuQ35PkgCfgData.csv'),
                          "Platform Hardcoded"
        )
        self.env.SetValue('CONF_PROFILE_NAMES', "P0,P1", "Platform Hardcoded")

        # Globally set CodeQL failures to be ignored in this repo.
        # Note: This has no impact if CodeQL is not active/enabled.
        self.env.SetValue("STUART_CODEQL_AUDIT_ONLY", "true", "Platform Defined")

        # Enabled all of the SMM modules
        self.env.SetValue("BLD_*_SMM_ENABLED", "TRUE", "Default")

        return 0

    def SetPlatformEnvAfterTarget(self):
        logging.debug("PlatformBuilder SetPlatformEnvAfterTarget")
        if os.name == 'nt':
            self.env.SetValue("VIRTUAL_DRIVE_PATH", Path(self.env.GetValue("BUILD_OUTPUT_BASE"), "VirtualDrive.vhd"), "Platform Hardcoded.")
        else:
            self.env.SetValue("VIRTUAL_DRIVE_PATH", Path(self.env.GetValue("BUILD_OUTPUT_BASE"), "VirtualDrive.img"), "Platform Hardcoded.")

        return 0

    def PlatformPreBuild(self):
        # Here we build the secure policy blob for build system to use and add into the targeted FV
        policy_example_dir = self.edk2path.GetAbsolutePathOnThisSystemFromEdk2RelativePath("MmSupervisorPkg", "SupervisorPolicyTools", "MmIsolationPoliciesExample.xml")
        output_dir = os.path.join(self.env.GetValue("BUILD_OUTPUT_BASE"), "Policy")
        if (not os.path.isdir(output_dir)):
            os.makedirs (output_dir)
        output_name = os.path.join(output_dir, "secure_policy.bin")

        ret = self.Helper.MakeSupervisorPolicy(xml_file_path=policy_example_dir, output_binary_path=output_name)
        if(ret != 0):
            raise Exception("SupervisorPolicyMaker Failed: Errorcode %d" % ret)
        self.env.SetValue("BLD_*_POLICY_BIN_PATH", output_name, "Set generated secure policy path")
        return ret

    # TODO: Validation should be done by parsing the cpu.c file from qemu
    def __ValidateCpuModelInfo(self):
        output_file = os.path.join(self.ws, "Build", "BUILDLOG_" +  self.GetName() + ".txt")
        cpu_brandname_dict = {
            "phenom": "AMD Phenom(tm) 9550 Quad-Core Processor",
            "coreduo": "Genuine Intel(R) CPU           T2600  @ 2.16GHz",
            "core2duo": "Intel(R) Core(TM)2 Duo CPU     T7700  @ 2.40GHz",
            "Skylake-Client-v1": "Intel Core Processor (Skylake)",
            "Skylake-Client-v2": "Intel Core Processor (Skylake, IBRS)",
            "Skylake-Client-v3": "Intel Core Processor (Skylake, IBRS, no TSX)",
            "Skylake-Client-v4": "Intel Core Processor (Skylake)",
        }

        cpu_model = self.env.GetValue("CPU_MODEL")
        cpu_brandname_log = 'CPU Brand Name:'

        with open(output_file, 'r') as handle:
            logs = handle.readlines()
            for line in logs:
                if cpu_brandname_log in line:
                    cpu_brandname = line.split(cpu_brandname_log)[-1].strip()

                    if cpu_brandname_dict[cpu_model] == cpu_brandname:
                        logging.critical("CPU brandname matches")
                        return 0

        # If the right logs are not found
        logging.error("CPU branding logs missing or incorrect")
        return -1


    def __SetEsrtGuidVars(self, var_name, guid_str, desc_string):
        cur_guid = uuid.UUID(guid_str)
        self.env.SetValue("BLD_*_%s_REGISTRY" % var_name, guid_str, desc_string)
        self.env.SetValue("BLD_*_%s_BYTES" % var_name, "'{" + (",".join(("0x%X" % byte) for byte in cur_guid.bytes_le)) + "}'", desc_string)
        return

    def FlashRomImage(self):
        run_tests = (self.env.GetValue("RUN_TESTS", "FALSE").upper() == "TRUE")
        output_base = self.env.GetValue("BUILD_OUTPUT_BASE")
        shutdown_after_run = (self.env.GetValue("SHUTDOWN_AFTER_RUN", "FALSE").upper() == "TRUE")
        empty_drive = (self.env.GetValue("EMPTY_DRIVE", "FALSE").upper() == "TRUE")
        test_regex = self.env.GetValue("TEST_REGEX", "")
        drive_path = self.env.GetValue("VIRTUAL_DRIVE_PATH")
        run_paging_audit = False

        # General debugging information for users
        if run_tests:
            if test_regex == "":
                logging.warning("Running tests, but no Tests specified. use TEST_REGEX to specify tests to run.")

            if not empty_drive:
                logging.info("EMPTY_DRIVE=FALSE. Old files can persist, could effect test results.")

            if not shutdown_after_run:
                logging.info("SHUTDOWN_AFTER_RUN=FALSE. You will need to close qemu manually to gather test results.")

        # Get a reference to the virtual drive, creating / wiping as necessary
        # Helper located at QemuPkg/Plugins/VirtualDriveManager
        virtual_drive = self.Helper.get_virtual_drive(drive_path)
        if empty_drive:
            virtual_drive.wipe()

        if not virtual_drive.exists():
            virtual_drive.make_drive()

        # Add tests if requested, auto run if requested
        # Creates a startup script with the requested tests
        if test_regex != "":
            test_list = []
            for pattern in test_regex.split(","):
                test_list.extend(Path(output_base, "X64").glob(pattern))

            if any("DxePagingAuditTestApp.efi" in os.path.basename(test) for test in test_list):
                run_paging_audit = True

            self.Helper.add_tests(virtual_drive, test_list, auto_run = run_tests, auto_shutdown = shutdown_after_run, paging_audit = run_paging_audit)
        # Otherwise add an empty startup script
        else:
            virtual_drive.add_startup_script([], auto_shutdown=shutdown_after_run)

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

        # Run Qemu
        # Helper located at Platforms/QemuQ35Pkg/Plugins/QemuRunner
        ret = self.Helper.QemuRun(self.env)
        if ret != 0:
            logging.critical("Failed running Qemu")
            return ret

        if self.env.GetValue("CPU_MODEL") is not None:
            self.__ValidateCpuModelInfo()

        if not run_tests:
            return 0

        # Gather test results if they were run.
        now = datetime.datetime.now()
        FET = FAILURE_EXEMPT_TESTS
        FEOL = FAILURE_EXEMPT_OMISSION_LENGTH

        if run_paging_audit:
            self.Helper.generate_paging_audit (virtual_drive, Path(drive_path).parent / "unit_test_results", self.env.GetValue("VERSION"), "Q35")

        # Filter out tests that are exempt
        tests = list(filter(lambda file: file.name not in FET or not (now - FET.get(file.name)).total_seconds() < FEOL, test_list))
        tests_exempt = list(filter(lambda file: file.name in FET and (now - FET.get(file.name)).total_seconds() < FEOL, test_list))
        if len(tests_exempt) > 0:
            self.Helper.report_results(virtual_drive, tests_exempt, Path(drive_path).parent / "unit_test_results")
        # Helper located at QemuPkg/Plugins/VirtualDriveManager
        return self.Helper.report_results(virtual_drive, tests, Path(drive_path).parent / "unit_test_results")

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
