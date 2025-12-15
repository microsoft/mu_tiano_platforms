# @file
# Script to Build QEMU SBSA Platform UEFI firmware
#
# Copyright (c) Microsoft Corporation.
# SPDX-License-Identifier: BSD-2-Clause-Patent
##
import datetime
import logging
import os
import re
import sys
from typing import Tuple
import uuid
from io import StringIO
from pathlib import Path
import json
import shutil

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

cached_enivron = os.environ.copy()

# Declare test whose failure will not return a non-zero exit code
FAILURE_EXEMPT_TESTS = {
    # example "PiValueTestApp.efi": datetime.datetime(3141, 5, 9, 2, 6, 53, 589793),
    "LineParserTestApp.efi": datetime.datetime(2025, 6, 2, 0, 0, 0, 0)
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
        "Features/CONFIG",
        "Features/FFA",
    )

    @staticmethod
    def add_common_command_line_options(parserObj) -> None:
        """Add common command line options to the parser object."""

    @staticmethod
    def get_common_command_line_options(settings, args) -> None:
        """Retrieves command line options common to settings managers."""

    @staticmethod
    def get_active_scopes() -> Tuple[str]:
        scopes = CommonPlatform.Scopes

        actual_tool_chain_tag = shell_environment.GetBuildVars().GetValue(
                "TOOL_CHAIN_TAG", ""
            )
        if actual_tool_chain_tag.upper().startswith("GCC"):
            scopes += ("gcc_aarch64_linux",)
        return scopes

    # ####################################################################################### #
    #                         Configuration for Update & Setup                                #
    # ####################################################################################### #
class SettingsManager(UpdateSettingsManager, SetupSettingsManager, PrEvalSettingsManager, ParseSettingsManager):
    def AddCommandLineOptions(self, parserObj):
        CommonPlatform.add_common_command_line_options(parserObj)

    def RetrieveCommandLineOptions(self, args):
        CommonPlatform.get_common_command_line_options(self, args)

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
            RequiredSubmodule("MU_BASECORE", False, ".pytool/CISettings.py"),
            RequiredSubmodule("Common/MU", False, ".pytool/CISettings.py"),
            RequiredSubmodule("Common/MU_TIANO", False, ".pytool/CISettings.py"),
            RequiredSubmodule("Common/MU_OEM_SAMPLE", False, ".pytool/CISettings.py"),
            RequiredSubmodule("Silicon/Arm/MU_TIANO", False, ".pytool/CISettings.py"),
            RequiredSubmodule("Silicon/Arm/TFA", True),
            RequiredSubmodule("Silicon/Arm/HAF", True),
            RequiredSubmodule("Features/DEBUGGER", True),
            RequiredSubmodule("Features/DFCI", True),
            RequiredSubmodule("Features/CONFIG", True),
            RequiredSubmodule("Features/FFA", True),
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
        return CommonPlatform.get_active_scopes()

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

    def AddCommandLineOptions(self, parserObj):
        CommonPlatform.add_common_command_line_options(parserObj)

    def RetrieveCommandLineOptions(self, args):
        CommonPlatform.get_common_command_line_options(self, args)

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

        # Also for the hafnium, do not check for the return code as it is not a fatal error
        cmd = "make"
        args = "clean"
        RunCmd(cmd, args, workingdir= self.env.GetValue("ARM_HAF_PATH"))

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
        return CommonPlatform.get_active_scopes()

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
        self.env.SetValue("ARM_TFA_PATH", str(Path(self.GetWorkspaceRoot()) / "Silicon" / "Arm" / "TFA"), "Platform hardcoded")
        self.env.SetValue("ARM_HAF_PATH", str(Path(self.GetWorkspaceRoot()) / "Silicon" / "Arm" / "HAF"), "Platform hardcoded")
        self.env.SetValue("BLD_*_QEMU_CORE_NUM", "4", "Default")
        self.env.SetValue("BLD_*_MEMORY_PROTECTION", "TRUE", "Default")
        # Include the MFCI test cert by default, override on the commandline with "BLD_*_SHIP_MODE=TRUE" if you want the retail MFCI cert
        self.env.SetValue("BLD_*_SHIP_MODE", "FALSE", "Default")
        self.env.SetValue("CONF_AUTOGEN_INCLUDE_PATH", self.edk2path.GetAbsolutePathOnThisSystemFromEdk2RelativePath("QemuSbsaPkg", "Include"), "Platform Defined")
        self.env.SetValue("MU_SCHEMA_DIR", self.edk2path.GetAbsolutePathOnThisSystemFromEdk2RelativePath("QemuSbsaPkg", "CfgData"), "Platform Defined")
        self.env.SetValue("MU_SCHEMA_FILE_NAME", "QemuSbsaPkgCfgData.xml", "Platform Hardcoded")
        self.env.SetValue("HAF_TFA_BUILD", "TRUE", "Platform Hardcoded")

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

    #
    # Update the transfer list checksum after patching content within a TL package.
    # The transfer list header has checksum at offset 4, and the checksum covers
    # all bytes from the TL base to the end of the used size.
    #
    def UpdateTransferListChecksum(self, file_name, tl_offset):
        """
        Update the transfer list checksum in a fip.bin file.

        Args:
            file_name: Path to the fip.bin file
            tl_offset: Offset of the transfer list header in the file

        Reference: Silicon/Arm/TFA/include/lib/transfer_list.h
        The transfer_list_header struct layout:
            uint32_t signature;   // offset 0, size 4
            uint8_t  checksum;    // offset 4, size 1
            uint8_t  version;     // offset 5, size 1
            uint8_t  hdr_size;    // offset 6, size 1
            uint8_t  alignment;   // offset 7, size 1
            uint32_t size;        // offset 8, size 4 (TL header + all TEs)
            uint32_t max_size;    // offset 12, size 4
            uint32_t flags;       // offset 16, size 4
            uint32_t reserved;    // offset 20, size 4
            Total header size: 24 bytes (0x18)
        """
        # Constants from TF-A transfer_list.h
        # See: Silicon/Arm/TFA/include/lib/transfer_list.h
        #   #define TRANSFER_LIST_SIGNATURE U(0x4a0fb10b)
        TL_SIGNATURE = 0x4A0FB10B

        # Offsets derived from struct transfer_list_header in transfer_list.h
        TL_HEADER_SIZE = 0x18          # Total size of transfer_list_header (24 bytes)
        TL_SIGNATURE_OFFSET = 0        # uint32_t signature at offset 0
        TL_SIGNATURE_SIZE = 4          # signature is 4 bytes
        TL_CHECKSUM_OFFSET = 4         # uint8_t checksum at offset 4
        TL_SIZE_OFFSET = 8             # uint32_t size at offset 8 (used size: header + all TEs)
        TL_SIZE_FIELD_SIZE = 4         # size field is 4 bytes

        with open(file_name, "r+b") as file:
            # Read the TL header
            file.seek(tl_offset)
            header = file.read(TL_HEADER_SIZE)

            # Validate signature
            signature = int.from_bytes(
                header[TL_SIGNATURE_OFFSET:TL_SIGNATURE_OFFSET + TL_SIGNATURE_SIZE],
                'little'
            )
            if signature != TL_SIGNATURE:
                logging.error(f"Invalid transfer list signature: 0x{signature:x}, expected 0x{TL_SIGNATURE:x}")
                return -1

            # Get the used size (TL header + all TEs)
            used_size = int.from_bytes(
                header[TL_SIZE_OFFSET:TL_SIZE_OFFSET + TL_SIZE_FIELD_SIZE],
                'little'
            )
            logging.info(f"Transfer list used size: 0x{used_size:x}")

            # Read the entire TL content
            file.seek(tl_offset)
            tl_data = bytearray(file.read(used_size))

            # First, zero out the checksum byte for calculation
            # Calculate new checksum (sum of all bytes mod 256, then 256 - sum)
            # Ensures new_checksum is uint8
            # This is the same calculation as calc_byte_sum in TFA transfer_list.c
            tl_data[TL_CHECKSUM_OFFSET] = 0
            byte_sum = sum(tl_data) % 256
            new_checksum = (256 - byte_sum) % 256

            logging.info(f"New transfer list checksum: 0x{new_checksum:x}")

            # Write the new checksum back
            file.seek(tl_offset + TL_CHECKSUM_OFFSET)
            file.write(bytes([new_checksum]))

        return 0

    def GetFipBlobOffsets(self, fip_path, fiptool_path):
        """
        Run fiptool info on fip.bin and parse the output to get UUID-to-offset/size mapping.

        Args:
            fip_path: Path to the fip.bin file
            fiptool_path: Path to the fiptool binary

        Returns:
            Dictionary mapping uppercase UUIDs to dict with 'offset' and 'size' in fip.bin
        """
        outstream = StringIO()
        ret = RunCmd(str(fiptool_path), f"info {fip_path}", outstream=outstream)
        if ret != 0:
            logging.error(f"Failed to run fiptool info on {fip_path}")
            return None

        uuid_to_info = {}
        output = outstream.getvalue()

        # Parse lines like:
        # EABA83D8-BAAF-4EAF-8144-F7FDCBE544A7: offset=0x54D03, size=0x283000, cmdline="--blob"
        # Also handle named entries like:
        # Trusted Boot Firmware BL2: offset=0x178, size=0x9B69, cmdline="--tb-fw"
        for line in output.splitlines():
            # Try to match UUID pattern (8-4-4-4-12 hex format) with offset and size
            uuid_match = re.match(r'^([0-9A-Fa-f]{8}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{12}):\s*offset=(0x[0-9A-Fa-f]+),\s*size=(0x[0-9A-Fa-f]+)', line)
            if uuid_match:
                blob_uuid = uuid_match.group(1).upper()
                offset = int(uuid_match.group(2), 16)
                size = int(uuid_match.group(3), 16)
                uuid_to_info[blob_uuid] = {'offset': offset, 'size': size}
                logging.debug(f"Found blob UUID {blob_uuid} at offset 0x{offset:x}, size 0x{size:x}")

        return uuid_to_info

    def SaveFipBlobManifest(self, uuid_to_info, manifest_path):
        """
        Save the FIP blob UUID-to-offset/size mapping to a JSON manifest file.

        Args:
            uuid_to_info: Dictionary mapping UUIDs to {'offset': int, 'size': int}
            manifest_path: Path to save the JSON manifest
        """
        with open(manifest_path, 'w') as f:
            json.dump(uuid_to_info, f, indent=4)
        logging.info(f"Saved FIP blob manifest to {manifest_path}")

    def LoadFipBlobManifest(self, manifest_path):
        """
        Load the FIP blob UUID-to-offset/size mapping from a JSON manifest file.

        Args:
            manifest_path: Path to the JSON manifest file

        Returns:
            Dictionary mapping uppercase UUIDs to {'offset': int, 'size': int}
        """
        if not manifest_path.exists():
            logging.error(f"FIP blob manifest not found at {manifest_path}")
            return None

        with open(manifest_path, 'r') as f:
            manifest_data = json.load(f)

        # Ensure UUIDs are uppercase for consistent lookup
        uuid_to_info = {}
        for uuid_key, info in manifest_data.items():
            uuid_to_info[uuid_key.upper()] = info
        logging.info(f"Loaded FIP blob manifest from {manifest_path}")
        return uuid_to_info

    def GetSpLayoutData(self):
        """
        Get the Secure Partition layout data.

        Returns:
            Dictionary containing the SP layout data
        """
        op_fv = Path(self.env.GetValue("BUILD_OUTPUT_BASE")) / "FV"

        # The SP layout structure - this matches what HafTfaBuild generates
        data = {
            "stmm": {
                "image": {
                    "file": str(op_fv / "BL32_AP_MM.fd"),
                    "offset": "0x2000"
                },
                "pm": {
                    "file": str(Path(__file__).parent / "fdts/qemu_sbsa_stmm_config.dts"),
                    "offset": "0x1000"
                },
                "package": "tl_pkg",
                "uuid": "eaba83d8-baaf-4eaf-8144-f7fdcbe544a7",
                "owner": "Plat",
                "size": "0x300000"
            },
            "mssp": {
                "image": {
                    "file": str(op_fv / "BL32_AP_MS_SP.fd"),
                    "offset": "0x10000"
                },
                "pm": {
                    "file": str(Path(__file__).parent / "fdts/qemu_sbsa_mssp_config.dts"),
                    "offset": "0x1000"
                },
                "uuid": "b8bcbd0c-8e8f-4ebe-99eb-3cbbdd0cd412",
                "owner": "Plat"
            },
            "mssp-rust": {
                "image": {
                    "file": str(Path(self.env.GetValue("SECURE_PARTITION_BINARIES")) / "msft-sp.bin"),
                    "offset": "0x2000"
                },
                "pm": {
                    "file": str(Path(__file__).parent / "fdts/qemu_sbsa_mssp_rust_config.dts"),
                    "offset": "0x1000"
                },
                "uuid": "AFF0C73B-47E7-4A5B-AFFC-0052305A6520",
                "owner": "Plat"
            }
        }

        return data

    def PatchSecurePartitions(self, op_tfa):
        """
        Copy fip.bin to a working directory and patch secure partition images into it.

        This function:
        1. Copies fip.bin from op_tfa to a working directory to avoid modifying originals
        2. Loads the FIP blob manifest to find blob offsets by UUID
        3. Iterates through the SP layout and patches each SP image at the correct offset
        4. Updates transfer list checksums for tl_pkg packages

        Args:
            op_tfa: Path to the TF-A output directory containing fip.bin and fip_blob_manifest.json

        Returns:
            Path to the patched working fip.bin on success, None on failure
        """
        # Copy fip.bin to the build output directory to avoid modifying the original files.
        # This way we don't accidentally corrupt the extdep files on subsequent runs.
        temp_fip_dir = Path(self.env.GetValue("BUILD_OUTPUT_BASE")) / "FIP"
        os.makedirs(temp_fip_dir, exist_ok=True)
        shutil.copy2(op_tfa / "fip.bin", temp_fip_dir / "fip.bin")
        working_fip = temp_fip_dir / "fip.bin"

        # Load UUID-to-offset/size mapping from the pre-generated manifest
        # This manifest is created during HAF_TFA_BUILD=TRUE and stored alongside the binaries
        manifest_path = op_tfa / "fip_blob_manifest.json"
        uuid_to_info = self.LoadFipBlobManifest(manifest_path)
        if uuid_to_info is None:
            return None

        # Get SP layout data
        sp_layout = self.GetSpLayoutData()

        # Iterate through SP layout and patch each SP image
        for sp_name, sp_config in sp_layout.items():
            sp_uuid = sp_config.get("uuid")
            if sp_uuid is None:
                logging.error(f"SP {sp_name} missing required 'uuid' field")
                return None
            sp_uuid = sp_uuid.upper()

            # Find the blob info in fip.bin for this UUID
            blob_info = uuid_to_info.get(sp_uuid)
            if blob_info is None:
                logging.error(f"UUID {sp_uuid} for SP {sp_name} not found in fip.bin")
                return None
            blob_offset = blob_info['offset']
            blob_size = blob_info['size']

            # Get the image file path and offset
            image_config = sp_config.get("image")
            if image_config is None:
                logging.error(f"SP {sp_name} missing required 'image' field")
                return None

            image_file_str = image_config.get("file")
            if image_file_str is None:
                logging.error(f"SP {sp_name} missing required 'image.file' field")
                return None
            image_file = Path(image_file_str)

            image_offset_str = image_config.get("offset")
            if image_offset_str is None:
                logging.error(f"SP {sp_name} missing required 'image.offset' field")
                return None
            image_offset = int(image_offset_str, 16)

            if not image_file.exists():
                logging.error(f"Image file {image_file} for SP {sp_name} not found")
                return None

            # For tl_pkg packages, the image offset needs to account for the pm offset
            # The actual image is at pm_offset + image_offset from the transfer list
            is_tl_pkg = sp_config.get("package") == "tl_pkg"
            pm_offset = 0
            if is_tl_pkg:
                pm_config = sp_config.get("pm")
                if pm_config is None:
                    logging.error(f"SP {sp_name} is tl_pkg but missing required 'pm' field")
                    return None
                pm_offset_str = pm_config.get("offset")
                if pm_offset_str is None:
                    logging.error(f"SP {sp_name} missing required 'pm.offset' field")
                    return None
                pm_offset = int(pm_offset_str, 16)

            final_offset = blob_offset + pm_offset + image_offset

            if os.stat(image_file).st_size > (blob_size - pm_offset):
                logging.error(f"Image file {image_file} size exceeds allocated blob size for SP {sp_name}. Must build HAF/TFA locally. Use HAF_TFA_BUILD=TRUE.")
                return None

            logging.info(f"Patching SP {sp_name} (UUID: {sp_uuid}) at offset 0x{final_offset:x}")
            logging.info(f"  Blob size: 0x{blob_size:x}, Image file: {image_file}, size: {os.stat(image_file).st_size}")

            ret = self.PatchRegion(
                working_fip,
                final_offset,
                os.stat(image_file).st_size,
                image_file,
            )
            if ret != 0:
                return None

            # Update transfer list checksum if this is a tl_pkg
            if is_tl_pkg:
                logging.info(f"Updating transfer list checksum for SP {sp_name}")
                ret = self.UpdateTransferListChecksum(working_fip, blob_offset)
                if ret != 0:
                    return None

        return working_fip

    def HafTfaBuild(self):
        logging.info("Starting Hafnium and TF-A build")
        src_dir = Path(self.GetWorkspaceRoot()) / "Platforms/QemuSbsaPkg/mu"
        dest_dir = Path(self.GetWorkspaceRoot()) / "Silicon/Arm/HAF/project/mu"

        # Remove the directory if it exists
        if dest_dir.exists():
            shutil.rmtree(dest_dir)

        # Copy the mu directory and its contents
        logging.info("Copying mu directory to Silicon/Arm/HAF/project")
        shutil.copytree(src_dir, dest_dir)

        # Add a post build step to build BL31 and assemble the FD files
        op_fv = Path(self.env.GetValue("BUILD_OUTPUT_BASE")) / "FV"

        logging.info("Building Hafnium")
        haf_out = Path(self.env.GetValue("BUILD_OUTPUT_BASE")) / "HAF"
        cmd = "make"
        args = "PROJECT=mu PLATFORM=secure_qemu_aarch64"
        args += " OUT=" + str(haf_out)
        ret = RunCmd(cmd, args, workingdir=self.env.GetValue("ARM_HAF_PATH"))
        if ret != 0:
            return ret

        logging.info("Building TF-A")

        shell_environment.CheckpointBuildVars()  # checkpoint our config before we mess with it
        if self.env.GetValue("TOOL_CHAIN_TAG") == "CLANGPDB":
            if os.name == "nt":
                # If this is a Windows build, we need to demolish the path and inject the VC variables of interest
                # otherwise the build could pick up wrong tools
                shell_environment.GetEnvironment().set_path("")
                self.InjectVcVarsOfInterests(["LIB", "Path"])

                clang_exe = "clang.exe"
                choco_path = shell_environment.GetEnvironment().get_shell_var("CHOCOLATEYINSTALL")
                shell_environment.GetEnvironment().insert_path(str(Path(choco_path) / "bin"))
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

        # Specify the filename
        filename = Path(self.env.GetValue("BUILD_OUTPUT_BASE")) / "sp_layout.json"

        # Writing JSON data
        with open(filename, "w") as f:
            json.dump(self.GetSpLayoutData(), f, indent=4)

        # This is an unorthodox build, as TF-A uses poetry to manage dependencies and build the firmware.
        # First, we need to know what the name of the virtual environment is.
        # This is stored in the poetry.lock file in the root of the TF-A directory.
        virtual_env = ""
        if sys.base_prefix != sys.prefix:
            # If we are in a virtual environment, we need to activate it before we can build the firmware.
            virtual_env = Path(sys.prefix) / "bin" / "activate"
            if not virtual_env.exists():
                logging.error("Virtual environment not found")
                return -1

        # Second, put together the command to build the firmware.
        cmd = "make"
        if self.env.GetValue("TOOL_CHAIN_TAG") == "CLANGPDB":
            args = "CC=" + clang_exe
        elif self.env.GetValue("TOOL_CHAIN_TAG") == "GCC5":
            args = "CROSS_COMPILE=" + shell_environment.GetEnvironment().get_shell_var("GCC5_AARCH64_PREFIX")
            args += " -j $(nproc)"
        else:
            logging.error("Unsupported toolchain")
            return -1
        args += " PLAT=" + self.env.GetValue("QEMU_PLATFORM").lower()
        args += " ARCH=" + self.env.GetValue("TARGET_ARCH").lower()
        args += " DEBUG=" + str(1 if self.env.GetValue("TARGET").lower() == 'debug' else 0)
        args += " ENABLE_SME_FOR_SWD=0 ENABLE_SVE_FOR_SWD=0 ENABLE_SME_FOR_NS=0 ENABLE_SVE_FOR_NS=0"
        args += f" SPD=spmd SPMD_SPM_AT_SEL2=1 SP_LAYOUT_FILE={filename}"
        args += " ENABLE_FEAT_HCX=1 HOB_LIST=1 TRANSFER_LIST=1 LOG_LEVEL=40" # Features used by hypervisor
        # args += " FEATURE_DETECTION=1" # Enforces support for features enabled.
        args += f" BL32={str(haf_out / 'secure_qemu_aarch64_clang' / 'hafnium.bin')}"
        args += " all fip"

        # Third, write a temp bash file to activate the virtual environment and build the firmware.
        temp_bash = Path(self.env.GetValue("BUILD_OUTPUT_BASE")) / "temp.sh"
        with open(temp_bash, "w") as f:
            f.write("#!/bin/bash\n")
            f.write("poetry --verbose install\n")
            f.write("poetry env activate\n")
            f.write("poetry show\n")
            f.write(f"{cmd} {args}\n")

        # Grab the current head to restore from patches later.
        patch_tfa = (self.env.GetValue("PATCH_TFA", "TRUE").upper() == "TRUE")
        if patch_tfa:
            outstream = StringIO()
            ret = RunCmd("git", "rev-parse HEAD", outstream=outstream, workingdir=self.env.GetValue("ARM_TFA_PATH"))
            if ret != 0:
                logging.error("Failed to get git HEAD for TFA")
                return ret
            arm_tfa_git_head = outstream.getvalue().strip()
            logging.info(f"TFA HEAD: {arm_tfa_git_head}")

            patches = str(Path(self.GetWorkspaceRoot()) / "Platforms/QemuSbsaPkg/tfa_patches" / "*.patch")
            # Log the patch files for debugging
            ret = RunCmd("git", f"am {patches}", workingdir=self.env.GetValue("ARM_TFA_PATH"), environ=cached_enivron)
            if ret != 0:
                return ret

        # Fifth, run the temp bash file to build the firmware.
        ret = RunCmd("bash", str(temp_bash), workingdir=self.env.GetValue("ARM_TFA_PATH"), environ=cached_enivron)
        if patch_tfa:
            # Always revert before returning
            revert_ret = RunCmd("git", f"checkout {arm_tfa_git_head}", workingdir=self.env.GetValue("ARM_TFA_PATH"), environ=cached_enivron)
            if revert_ret != 0:
                return revert_ret

        if ret != 0:
            return ret

        # Fourth, remove the temp bash file, if succeeded.
        os.remove(temp_bash)

        # Revert the build vars to the original state
        shell_environment.RevertBuildVars()

        # Create output directory to store all Hafnium and TFA bins
        output_dir = Path(self.env.GetValue("BUILD_OUTPUT_BASE")) / "HafTfaBins"
        os.makedirs(output_dir, exist_ok=True)

        # Copy firmware binaries

        # Collect hafnium.bin explicitly
        hafnium_bin = haf_out / "secure_qemu_aarch64_clang" / "hafnium.bin"
        if not hafnium_bin.exists():
            logging.error(f"Hafnium binary not found at {hafnium_bin}")
            return -1
        shutil.copy2(hafnium_bin, output_dir)
        logging.debug(f"{hafnium_bin} saved to: {output_dir}")

        # Copy any *.bin artifacts from op_tfa
        op_tfa = Path(self.env.GetValue("ARM_TFA_PATH")) / "build" / self.env.GetValue("QEMU_PLATFORM").lower() / self.env.GetValue("TARGET").lower()
        for bin_file in op_tfa.glob("*.bin"):
            try:
                shutil.copy2(bin_file, output_dir)
                logging.debug(f"{bin_file} saved to: {output_dir}")
            except Exception as e:
                logging.error(f"Failed to copy {bin_file}: {e}")
                return -1

        fiptool_path = Path(self.env.GetValue("ARM_TFA_PATH")) / "tools" / "fiptool" / "fiptool"
        if not fiptool_path.exists():
            logging.error(f"Fiptool binary not found at {fiptool_path}")
            return -1

        # Generate FIP blob manifest for use when HAF_TFA_BUILD=FALSE
        # This allows local builds to patch SPs without needing to run fiptool
        fip_path = op_tfa / "fip.bin"
        uuid_to_info = self.GetFipBlobOffsets(fip_path, fiptool_path)
        if uuid_to_info:
            self.SaveFipBlobManifest(uuid_to_info, output_dir / "fip_blob_manifest.json")
        else:
            logging.error("Failed to generate FIP blob manifest - patching may not work with HAF_TFA_BUILD=FALSE")
            return -1

        logging.debug(f"Copied all Hafnium and TFA binaries to {output_dir}")
        return 0

    def PlatformPostBuild(self):
        if self.env.GetValue("HAF_TFA_BUILD") == "TRUE":
            ret = self.HafTfaBuild()
            if ret != 0:
                return ret
            op_tfa = Path(self.env.GetValue("ARM_TFA_PATH")) / "build" / self.env.GetValue("QEMU_PLATFORM").lower() / self.env.GetValue("TARGET").lower()
            working_fip = op_tfa / "fip.bin"
        else:
            ext_dep_bins = self.env.GetValue("HAF_TFA_BINS")
            if not ext_dep_bins:
                logging.error("HAF_TFA_BINS not set. Cannot patch secure partitions.")
                return -1
            op_tfa = Path(ext_dep_bins)
            working_fip = self.PatchSecurePartitions(op_tfa) # Patch secure partition images into a working copy of fip.bin
            if working_fip is None:
                return -1

        # Now that BL31 is built with BL32 supplied, patch BL1 and BL31 built fip.bin into the SECURE_FLASH0.fd
        # Add a post build step to build BL31 and assemble the FD files
        op_fv = Path(self.env.GetValue("BUILD_OUTPUT_BASE")) / "FV"

        logging.info("Patching BL1 region")
        ret = self.PatchRegion(
            op_fv / "SECURE_FLASH0.fd",
            int(self.env.GetValue("SECURE_FLASH_REGION_BL1_OFFSET"), 16),
            int(self.env.GetValue("SECURE_FLASH_REGION_BL1_SIZE"), 16),
            op_tfa / "bl1.bin",
        )
        if ret != 0:
            return ret

        logging.info("Patching FIP region")
        ret = self.PatchRegion(
            op_fv / "SECURE_FLASH0.fd",
            int(self.env.GetValue("SECURE_FLASH_REGION_FIP_OFFSET"), 16),
            int(self.env.GetValue("SECURE_FLASH_REGION_FIP_SIZE"), 16),
            working_fip,
        )
        if ret != 0:
            return ret

        # Pad both fd to 256mb, as required by QEMU
        OutputPath_FV = Path(self.env.GetValue("BUILD_OUTPUT_BASE")) / "FV"
        Built_FV = OutputPath_FV / "QEMU_EFI.fd"
        with open(Built_FV, "ab") as fvfile:
            fvfile.seek(0, os.SEEK_END)
            additional = b'\0' * ((256 * 1024 * 1024)-fvfile.tell())
            fvfile.write(additional)

        bl3 = OutputPath_FV / "SECURE_FLASH0.fd"
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
