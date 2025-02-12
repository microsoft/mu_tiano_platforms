# @file
# Script to Build QEMU SBSA Platform UEFI firmware
#
# Copyright (c) Microsoft Corporation.
# SPDX-License-Identifier: BSD-2-Clause-Patent
##
import datetime
import logging
import os
import uuid
from io import StringIO
from pathlib import Path

from edk2toolext.environment import shell_environment
from edk2toolext.environment.uefi_build import UefiBuilder
from edk2toolext.invocables.edk2_platform_build import BuildSettingsManager
from edk2toolext.invocables.edk2_pr_eval import PrEvalSettingsManager
from edk2toolext.invocables.edk2_setup import (RequiredSubmodule,
                                               SetupSettingsManager)
from edk2toolext.invocables.edk2_update import UpdateSettingsManager
from edk2toolext.invocables.edk2_parse import ParseSettingsManager
from edk2toollib.utility_functions import RunCmd
from edk2toollib.windows.locate_tools import QueryVcVariables
from edk2toollib.utility_functions import GetHostInfo

# Declare test whose failure will not return a non-zero exit code
FAILURE_EXEMPT_TESTS = {
    # example "PiValueTestApp.efi": datetime.datetime(3141, 5, 9, 2, 6, 53, 589793),
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
    PackagesSupported = ("QemuSbsaPkg",)
    ArchSupported = ("AARCH64",)
    TargetsSupported = ("DEBUG", "RELEASE", "NOOPT")
    Scopes = ('qemu', 'qemusbsa', 'edk2-build', 'cibuild', 'configdata')
    WorkspaceRoot = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    PackagesPath = (
        "Platforms",
        "MU_BASECORE",
        "Common/MU",
        "Common/MU_TIANO",
        "Common/MU_OEM_SAMPLE",
        "Silicon/Arm/MU_TIANO",
        "Silicon/Arm/TFA",
        "Features/DEBUGGER",
        "Features/DFCI",
        "Features/CONFIG"
    )


    # ####################################################################################### #
    #                         Configuration for Update & Setup                                #
    # ####################################################################################### #
class SettingsManager(UpdateSettingsManager, SetupSettingsManager, PrEvalSettingsManager, ParseSettingsManager):

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
            RequiredSubmodule("Silicon/Arm/MU_TIANO", True),
            RequiredSubmodule("Silicon/Arm/TFA", True),
            RequiredSubmodule("Features/DEBUGGER", True),
            RequiredSubmodule("Features/DFCI", True),
            RequiredSubmodule("Features/CONFIG", True),
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
        scopes = CommonPlatform.Scopes
        actual_tool_chain_tag = shell_environment.GetBuildVars().GetValue(
                "TOOL_CHAIN_TAG", ""
            )
        if actual_tool_chain_tag.upper().startswith("GCC"):
            scopes += ("gcc_aarch64_linux",)
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
        return ("QemuSbsaPkg/QemuSbsaPkg.dsc", {})

    def GetName(self):
        return "QemuSbsa"

    def GetPackagesPath(self):
        ''' Return a list of paths that should be mapped as edk2 PackagesPath '''
        return CommonPlatform.PackagesPath

    # ####################################################################################### #
    #                         Actual Configuration for Platform Build                         #
    # ####################################################################################### #
class PlatformBuilder(UefiBuilder, BuildSettingsManager):
    def __init__(self):
        UefiBuilder.__init__(self)

    # Helper function to query the VC variables of interest and inject them into the environment
    def InjectVcVarsOfInterests(self, vcvars: list):
        HostInfo = GetHostInfo()

        # check to see if host is configured
        # HostType for VS tools should be (defined in tools_def):
        # x86   == 32bit Intel
        # x64   == 64bit Intel
        # arm   == 32bit Arm
        # arm64 == 64bit Arm
        #
        HostType = shell_environment.GetEnvironment().get_shell_var("CLANG_VS_HOST")
        if HostType is not None:
            HostType = HostType.lower()
            logging.info(
                f"CLANG_VS_HOST defined by environment.  Value is {HostType}")
        else:
            #figure it out based on host info
            if HostInfo.arch == "x86":
                if HostInfo.bit == "32":
                    HostType = "x86"
                elif HostInfo.bit == "64":
                    HostType = "x64"
            else:
                # anything other than x86 or x64 is not supported
                raise NotImplementedError()

        # CLANG_VS_HOST options are not exactly the same as QueryVcVariables. This translates.
        VC_HOST_ARCH_TRANSLATOR = {
            "x86": "x86", "x64": "AMD64", "arm": "not supported", "arm64": "not supported"}

        # now get the environment variables for the platform
        shell_env = shell_environment.GetEnvironment()
        # Use the tools lib to determine the correct values for the vars that interest us.
        vs_vars = QueryVcVariables(
            vcvars, VC_HOST_ARCH_TRANSLATOR[HostType])
        for (k, v) in vs_vars.items():
            shell_env.set_shell_var(k, v)

    def CleanTree(self, RemoveConfTemplateFilesToo=False):
        # If this is a Windows Clang build, we need to inject the VC variables of interest
        if self.env.GetValue("TOOL_CHAIN_TAG") == "CLANGPDB" and os.name == 'nt':
            self.InjectVcVarsOfInterests(["VCToolsInstallDir", "Path", "LIB"])

        # Add a step to clean up BL31 as well, if asked
        cmd = "make"
        args = "distclean"
        RunCmd(cmd, args, workingdir=self.env.GetValue("ARM_TFA_PATH"))

        return super().CleanTree(RemoveConfTemplateFilesToo)

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
        scopes = CommonPlatform.Scopes
        actual_tool_chain_tag = shell_environment.GetBuildVars().GetValue(
                "TOOL_CHAIN_TAG", ""
            )
        if actual_tool_chain_tag.upper().startswith("GCC"):
            scopes += ("gcc_aarch64_linux",)
        return scopes

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
        """Get the logging level depending on logger type.

        Args:
            loggerType (str): type of logger being logged to

        Returns:
            (Logging.Level): The logging level

        !!! note "loggerType possible values"
            "base": lowest logging level supported

            "con": logs to screen

            "txt": logs to plain text file
        """
        return logging.INFO
        return super().GetLoggingLevel(loggerType)

    def SetPlatformEnv(self):
        logging.debug("PlatformBuilder SetPlatformEnv")
        self.env.SetValue("PRODUCT_NAME", "QemuSbsa", "Platform Hardcoded")
        self.env.SetValue("ACTIVE_PLATFORM", "QemuSbsaPkg/QemuSbsaPkg.dsc", "Platform Hardcoded")
        self.env.SetValue("TARGET_ARCH", "AARCH64", "Platform Hardcoded")
        self.env.SetValue("TOOL_CHAIN_TAG", "GCC5", "set default to gcc5")
        self.env.SetValue("EMPTY_DRIVE", "FALSE", "Default to false")
        self.env.SetValue("RUN_TESTS", "FALSE", "Default to false")
        self.env.SetValue("QEMU_HEADLESS", "FALSE", "Default to false")
        self.env.SetValue("QEMU_PLATFORM", "qemu_sbsa", "Platform Hardcoded")
        self.env.SetValue("SHUTDOWN_AFTER_RUN", "FALSE", "Default to false")
        # needed to make FV size build report happy
        self.env.SetValue("BLD_*_BUILDID_STRING", "Unknown", "Default")
        # Default turn on build reporting.
        self.env.SetValue("BUILDREPORTING", "TRUE", "Enabling build report")
        self.env.SetValue("BUILDREPORT_TYPES", "PCD DEPEX FLASH BUILD_FLAGS LIBRARY FIXED_ADDRESS HASH", "Setting build report types")
        self.env.SetValue("ARM_TFA_PATH", os.path.join (self.GetWorkspaceRoot (), "Silicon/Arm/TFA"), "Platform hardcoded")
        self.env.SetValue("BLD_*_QEMU_CORE_NUM", "4", "Default")
        self.env.SetValue("BLD_*_MEMORY_PROTECTION", "TRUE", "Default")
        # Include the MFCI test cert by default, override on the commandline with "BLD_*_SHIP_MODE=TRUE" if you want the retail MFCI cert
        self.env.SetValue("BLD_*_SHIP_MODE", "FALSE", "Default")
        self.env.SetValue("CONF_AUTOGEN_INCLUDE_PATH", self.edk2path.GetAbsolutePathOnThisSystemFromEdk2RelativePath("QemuSbsaPkg", "Include"), "Platform Defined")
        self.env.SetValue("MU_SCHEMA_DIR", self.edk2path.GetAbsolutePathOnThisSystemFromEdk2RelativePath("QemuSbsaPkg", "CfgData"), "Platform Defined")
        self.env.SetValue("MU_SCHEMA_FILE_NAME", "QemuSbsaPkgCfgData.xml", "Platform Hardcoded")

        if self.Helper.generate_secureboot_pcds(self) != 0:
            logging.error("Failed to generate include PCDs")
            return -1

        return 0

    def SetPlatformEnvAfterTarget(self):
        logging.debug("PlatformBuilder SetPlatformEnvAfterTarget")
        if os.name == 'nt':
            self.env.SetValue("VIRTUAL_DRIVE_PATH", Path(self.env.GetValue("BUILD_OUTPUT_BASE"), "VirtualDrive.vhd"), "Platform Hardcoded.")
        else:
            self.env.SetValue("VIRTUAL_DRIVE_PATH", Path(self.env.GetValue("BUILD_OUTPUT_BASE"), "VirtualDrive.img"), "Platform Hardcoded.")

        return 0

    def PlatformPreBuild(self):
        return 0

    #
    # Copy a file into the designated region of target FD.
    #
    def PatchRegion(self, fdfile, mainStart, size, srcfile):
        src_size = os.stat(srcfile).st_size
        if src_size > size:
            logging.error("Source file size is larger than the target region")
            return -1

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

        shell_environment.CheckpointBuildVars()  # checkpoint our config before we mess with it
        if self.env.GetValue("TOOL_CHAIN_TAG") == "CLANGPDB":
            if os.name == 'nt':
                # If this is a Windows build, we need to demolish the path and inject the VC variables of interest
                # otherwise the build could pick up wrong tools
                shell_environment.GetEnvironment().set_path('')
                self.InjectVcVarsOfInterests(["LIB", "Path"])

                clang_exe = "clang.exe"
                choco_path = shell_environment.GetEnvironment().get_shell_var("CHOCOLATEYINSTALL")
                shell_environment.GetEnvironment().insert_path(os.path.join(choco_path, "bin"))
                shell_environment.GetEnvironment().insert_path(shell_environment.GetEnvironment().get_shell_var("CLANG_BIN"))

                # Need to build fiptool separately because the build system will override LIB with LIBC for firmware builds
                cmd = "make"
                args = " fiptool MAKEFLAGS= LIB=\"" + shell_environment.GetEnvironment().get_shell_var("LIB") + "\""
                ret = RunCmd(cmd, args, workingdir=self.env.GetValue("ARM_TFA_PATH"))
                if ret != 0:
                    return ret
                # Then we can make the firmware images with the fiptool built above
            else:
                clang_exe = "clang"

        cmd = "make"
        if self.env.GetValue("TOOL_CHAIN_TAG") == "CLANGPDB":
            args = "CC="+clang_exe
        elif self.env.GetValue("TOOL_CHAIN_TAG") == "GCC5":
            args = "CROSS_COMPILE=" + shell_environment.GetEnvironment().get_shell_var("GCC5_AARCH64_PREFIX")
            args += " -j $(nproc)"
        else:
            logging.error("Unsupported toolchain")
            return -1
        args += " PLAT=" + self.env.GetValue("QEMU_PLATFORM").lower()
        args += " ARCH=" + self.env.GetValue("TARGET_ARCH").lower()
        args += " DEBUG=" + str(1 if self.env.GetValue("TARGET").lower() == 'debug' else 0)
        args += " SPM_MM=1 EL3_EXCEPTION_HANDLING=1 ENABLE_SME_FOR_NS=0 ENABLE_SVE_FOR_NS=0"
        args += " ENABLE_FEAT_HCX=1" # Features used by hypervisor
        # args += " FEATURE_DETECTION=1" # Enforces support for features enabled.
        args += " BL32=" + os.path.join(op_fv, "BL32_AP_MM.fd")
        args += " all fip"
        ret = RunCmd(cmd, args, workingdir= self.env.GetValue("ARM_TFA_PATH"))
        if ret != 0:
            return ret

        # Revert the build vars to the original state
        shell_environment.RevertBuildVars()

        # Now that BL31 is built with BL32 supplied, patch BL1 and BL31 built fip.bin into the SECURE_FLASH0.fd
        op_tfa = os.path.join (
            self.env.GetValue("ARM_TFA_PATH"), "build",
            self.env.GetValue("QEMU_PLATFORM").lower(),
            self.env.GetValue("TARGET").lower())

        logging.info("Patching BL1 region")
        ret = self.PatchRegion(
                os.path.join(op_fv, "SECURE_FLASH0.fd"),
                int(self.env.GetValue("SECURE_FLASH_REGION_BL1_OFFSET"), 16),
                int( self.env.GetValue("SECURE_FLASH_REGION_BL1_SIZE"), 16),
                os.path.join(op_tfa, "bl1.bin"),
                )
        if ret != 0:
            return ret

        logging.info("Patching FIP region")
        ret = self.PatchRegion(
                os.path.join(op_fv, "SECURE_FLASH0.fd"),
                int(self.env.GetValue("SECURE_FLASH_REGION_FIP_OFFSET"), 16),
                int( self.env.GetValue("SECURE_FLASH_REGION_FIP_SIZE"), 16),
                os.path.join(op_tfa, "fip.bin")
                )
        if ret != 0:
            return ret

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
                test_list.extend(Path(output_base, "AARCH64").glob(pattern))

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

        if not run_tests:
            return 0

        # Gather test results if they were run.
        now = datetime.datetime.now()
        FET = FAILURE_EXEMPT_TESTS
        FEOL = FAILURE_EXEMPT_OMISSION_LENGTH

        if run_paging_audit:
            self.Helper.generate_paging_audit (virtual_drive, Path(drive_path).parent / "unit_test_results", self.env.GetValue("VERSION"), "SBSA")

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

    from edk2toolext.invocables.edk2_platform_build import Edk2PlatformBuild
    from edk2toolext.invocables.edk2_setup import Edk2PlatformSetup
    from edk2toolext.invocables.edk2_update import Edk2Update
    print(r"Invoking Stuart")
    print(r"     ) _     _")
    print(r"    ( (^)-~-(^)")
    print(r"__,-.\_( 0 0 )__,-.___")
    print(r"  'W'   \   /   'W'")
    print(r"         >o<")
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
