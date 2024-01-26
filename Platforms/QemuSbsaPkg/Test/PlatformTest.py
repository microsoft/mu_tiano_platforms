# @file
# Script to Build QemuQSbsa host-based unit tests.
#
# Copyright (c) Microsoft Corporation.
# SPDX-License-Identifier: BSD-2-Clause-Patent
##
from collections import namedtuple
from edk2toolext.invocables.edk2_platform_build import BuildSettingsManager
from edk2toolext.environment.uefi_build import UefiBuilder
from edk2toollib.utility_functions import RunCmd
from edk2toolext.edk2_logging import SECTION, SUB_SECTION 
from edk2toollib.uefi.edk2.parsers.dsc_parser import DscParser
from edk2toollib.database import Edk2DB, Environment, Inf, Source, InstancedInf
from sqlalchemy import func, not_

from pathlib import Path
import logging
import sys
import io
import shutil
import re

sys.path.append(str(Path(__file__).parent.parent))
import PlatformBuild  # noqa: E402


PLATFORM_NAME = 'QemuSbsaPkg'
PLATFORM_TEST_DSC = 'QemuSbsaPkg/Test/QemuSbsaPkgHostTest.dsc'
PLATFORM_DSC = 'QemuSbsaPkg/QemuSbsaPkg.dsc'
PLATFORMBUILD_DIR = str(Path(__file__).parent.parent)


class TestSettingsManager(PlatformBuild.SettingsManager):
    pass


class TestManager(BuildSettingsManager, UefiBuilder):

    def GetLoggingLevel(self, loggerType):
        if loggerType == 'con':
            return logging.INFO
        return logging.DEBUG
    
    def GetWorkspaceRoot(self) -> str:
        return str(Path(__file__).parent.parent.parent.parent)
    
    def GetPackagesPath(self):
        return PlatformBuild.CommonPlatform.PackagesPath

    def GetActiveScopes(self):
        return ('qemu', 'qemusbsa', 'gcc_aarch64_linux', 'edk2-build', 'cibuild', 'host-based-test')
    
    def GetName(self):
        return f"{PLATFORM_NAME}_HostBasedTest"

    def SetPlatformEnv(self):
        logging.debug("PlatformBuilder SetPlatformEnv")
        self.env.SetValue("ACTIVE_PLATFORM", PLATFORM_TEST_DSC, "Platform Hardcoded.")
        self.env.SetValue("TARGET", "NOOPT", "Platform Hardcoded.")
        self.env.SetValue("CI_BUILD_TYPE", "host_unit_test", "Platform Hardcoded.")
        self.env.SetValue("TARGET_ARCH", "X64", "Platform Hardcoded.")
        self.env.SetValue("TOOL_CHAIN_TAG", "GCC5", "Platform Hardcoded.")

        # Don't let the host runner reorganize the build. This file will do it by platform.
        self.env.SetValue("CC_REORGANIZE", "FALSE", "Platform Hardcoded")

        # Must use PlatformFlashImage to generate coverage report as PlatformPostBuild runs before PostBuildPlugins,
        # Which generates intitial code coverage.
        if self.env.GetValue("CODE_COVERAGE") == "TRUE":
            self.FlashImage = True
        return 0
    
    def SetPlatformDefaultEnv(self):
        Env = namedtuple("Env", ["name", "default", "description"])

        return [
            Env("CODE_COVERAGE", "FALSE", "Generate Code Coverage Reports"),
            Env("REPORTTYPES", "Cobertura", "Code Coverage Report Types"),
            Env("CC_FLATTEN", "TRUE", "Group Coverage Results by source file instead of by INF."),
            Env("CC_FULL", "FALSE", "Create coverage lines for files without any coverage data.")
        ]

    def PlatformPreBuild(self):
        # Make sure code cov tools are installed if they want code coverage reports.
        if self.env.GetValue("CODE_COVERAGE") == "TRUE" and not self._verify_code_cov_tools():
            return -1

        # Parse the platform so we can verify the test dsc is up to date
        db_path = Path(self.GetWorkspaceRoot(), "Build", "DATABASE.db")
        if not self._parse_platform(db_path, PLATFORMBUILD_DIR):
            return -1

        if not self._verify_test_dsc(db_path):
            return -1

        logging.info("Host Based Tests are up to date.")

        return 0

    def PlatformFlashImage(self):
        reporttypes = self.env.GetValue("REPORTTYPES").split(",")
        logging.log(SECTION, "Generating Requested Code Coverage Reports")
        logging.info(f'Report Types: {",".join(reporttypes)}')

        coverage_file = Path(self.env.GetValue("BUILD_OUTPUT_BASE"), "_coverage.xml")
        coverage_file = str(coverage_file.replace(coverage_file.parent / f'{PLATFORM_NAME}_coverage.xml'))
        if not self._reorganize_coverage_report(coverage_file):
            return -1

        out_dir = Path(self.env.GetValue("BUILD_OUTPUT_BASE"), "Coverage")
        out_dir.mkdir(parents=True, exist_ok=True)
        if not self._generate_reports(coverage_file, out_dir, reporttypes):
            return -1

        return 0

    def _verify_test_dsc(self, db_path: Path) -> bool:
        """Compares all source files used by the platform to the source files used by the host based tests."""
        logging.log(SECTION, "Verify Host Based Tests are Up to Date")
        dscp = DscParser()
        dscp.SetEdk2Path(self.edk2path).SetInputVars(self.env.GetAllBuildKeyValues() | self.env.GetAllNonBuildKeyValues())
        dscp.ParseFile(self.env.GetValue("ACTIVE_PLATFORM")) # The test DSC

        with Edk2DB(db_path).session() as session:
            used_tests = set([component for component, _, _ in dscp.Components])
            env_id = session.query(Environment).filter(Environment.values.any(key="ACTIVE_PLATFORM", value=PLATFORM_DSC)).order_by(Environment.date.desc()).first().id
            host_tests = (
                session
                    .query(Inf.path, Source.path)
                    .join(Inf.sources)
                    .filter(Inf.module_type == "HOST_APPLICATION")
                    .all()
            )
            source_query = (
                session
                    .query(Source.path)
                    .join(InstancedInf.sources)
                    .filter(InstancedInf.env == env_id)
                    .filter(not_(func.lower(InstancedInf.name).like("%testapp%")))
            )
            used_source = set([source for source, in source_query.all()])
            must_use_tests = set([module for module, source in host_tests if source in used_source])

            unused_tests = must_use_tests - used_tests
        
        if len(unused_tests) > 0:
            logging.error("The following host based unit tests test files used by the "
                          "platform, but are not in the host based unit test dsc:")
            logging.error("\n  ".join(unused_tests))
            return False

        return True

    def _parse_platform(self, db_path: Path, platform_dir: str) -> bool:
        """Parses the platform if necessary."""
        if not db_path.exists() or not self._verify_db_data(db_path):
            try:
                logging.log(SUB_SECTION , "Running stuart_parse")
                RunCmd("stuart_parse", '', workingdir = platform_dir, logging_level=logging.DEBUG, raise_exception_on_nonzero=True)
            except Exception:
                logging.error("Failed to run stuart_parse. Review Build/PARSE_LOG.txt")
                return False
        else:
            logging.warning("Skipping Parse as database contains necessary data.")
        return True

    def _reorganize_coverage_report(self, cov_file: str) -> bool:
        """Reorganizes a coverage report by platform."""
        params = "coverage"
        params += f' {cov_file}'
        params += f' -ws {self.GetWorkspaceRoot()}'
        params += ' --by-platform'
        params += f' -d {PLATFORM_DSC}'
        params += f' -o {cov_file}'
        params += ' --full' * int(self.env.GetValue("CC_FULL") == "TRUE")
        params += ' --flatten' * int(self.env.GetValue("CC_FLATTEN") == "TRUE")
        try:
            RunCmd("stuart_report", params, logging_level=logging.DEBUG, raise_exception_on_nonzero = True)
        except Exception:
            logging.error("stuart_report Failed to generate a report.")
            return False
        return True
        
    def _generate_reports(self, cov_file: str, out_dir: str, reporttypes: list) -> bool:
        """Generates one or more coverage report types using reportgenerator."""
        if self.env.GetValue("REPORTTYPES") == 'Cobertura':
            shutil.copy2(cov_file, out_dir)
        else:
            params = f'-reports:"{cov_file}"'
            params += f' -targetdir:"{str(out_dir)}"'
            params += f' -reporttypes:{";".join(reporttypes)}'
            try:
                RunCmd("reportgenerator", params, logging_level=logging.DEBUG, raise_exception_on_nonzero = True)
            except Exception:
                logging.error("reportgenerator Failed to generate a report.")
                return False
        # Clean up the raw coverage file
        Path(cov_file).unlink()
        return True

    def _verify_code_cov_tools(self) -> bool:
        "Verifies if the necessary coverage tools are installed."
        COV_TOOLS = {
            "reportgenerator": ("-h", "Parameters"),
            "OpenCppCoverage": ("-h", "Command line only:"),
            "lcov": ("--help", "Options:"),
            "lcov_cobertura": ("-h", "Options:"),
        }

        # Register the tools to check
        tools = []
        if self.env.GetValue("REPORTTYPES") != "Cobertura":
            tools.append("reportgenerator")

        if sys.platform.startswith("win"):
            tools.append("OpenCppCoverage")
        else:
            tools.extend(["lcov", "lcov_cobertura"])

        # Check if the tools are installed
        for tool in tools:
            params, pattern = COV_TOOLS[tool]
            output = io.StringIO()
            RunCmd(tool, params, outstream=output, logging_level=logging.DEBUG)
            output.seek(0)
            match = re.search(pattern, output.getvalue())
            if not match:
                logging.error(f"You do not have {tool} installed, but current command settings require the tool.")
                return False
        return True

    def _verify_db_data(self, db_path: Path) -> bool:
        with Edk2DB(db_path).session() as session:
            if len(session.query(Environment).filter(Environment.values.any(key="ACTIVE_PLATFORM", value=PLATFORM_DSC)).all()) == 0:
                return False
            return True
