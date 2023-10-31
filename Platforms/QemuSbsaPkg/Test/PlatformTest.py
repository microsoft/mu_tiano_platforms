from edk2toolext.invocables.edk2_platform_build import BuildSettingsManager
from edk2toolext.environment.uefi_build import UefiBuilder
from edk2toollib.utility_functions import RunCmd
from edk2toolext.edk2_logging import SECTION, SUB_SECTION 
from edk2toollib.uefi.edk2.parsers.dsc_parser import DscParser
from edk2toollib.database import Edk2DB
from pathlib import Path
import logging
import sys

sys.path.append(str(Path(__file__).parent.parent))
import PlatformBuild  # noqa: E402


PLATFORM_DSC = 'QemuSbsaPkg/QemuSbsaPkg.dsc'
PLATFORMBUILD_DIR = str(Path(__file__).parent.parent)

# The query to determine which INFs test source files used by QemuQ35Pkg/QemuQ35Pkg.dsc
TEST_QUERY = """
WITH host_test_files AS (
    SELECT inf.path as 'inf', junction.key2 as 'source'
    FROM
        inf,
        junction
    WHERE
        junction.table1 = 'inf'
        AND junction.table2 = 'source'
        AND inf.module_type = 'HOST_APPLICATION'
    AND inf.path = junction.key1
)
SELECT DISTINCT host_test_files.inf
FROM
    instanced_inf_source_junction AS iisj
JOIN host_test_files ON iisj.source = host_test_files.source
WHERE iisj.instanced_inf NOT LIKE '%UnitTestApp.inf';
"""

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
        return ('qemu', 'qemusbsa', 'edk2-build', 'cibuild', 'host-based-test')
    
    def GetName(self):
        return "QemuSbsaPkg_HostBasedTest"

    def SetPlatformEnv(self):
        logging.debug("PlatformBuilder SetPlatformEnv")
        self.env.SetValue("ACTIVE_PLATFORM", "QemuSbsaPkg/Test/QemuSbsaPkgHostTest.dsc", "Platform Hardcoded.")
        self.env.SetValue("TARGET", "NOOPT", "Platform Hardcoded.")
        self.env.SetValue("CI_BUILD_TYPE", "host_unit_test", "Platform Hardcoded.")
        self.env.SetValue("TARGET_ARCH", "X64", "Platform Hardcoded.")
        self.env.SetValue("TOOL_CHAIN_TAG", "VS2022", "Platform Hardcoded.")
        self.env.SetValue("CODE_COVERAGE", "TRUE", "Platform Hardcoded.")
        self.env.SetValue("REPORTTYPES", "Cobertura", "Platform Hardcoded.")

        # Must use PlatformFlashImage to generate coverage report as PlatformPostBuild runs before PostBuildPlugins,
        # Which generates intitial code coverage.
        if self.env.GetValue("CODE_COVERAGE") == "TRUE":
            self.FlashImage = True
        return 0
    
    def PlatformPreBuild(self):
        # Make sure code cov tools are installed if they want code coverage reports.
        if self.env.GetValue("CODE_COVERAGE") == "TRUE" and not self._verify_code_cov_tools():
            return -1
            
        DB_PATH = Path(self.GetWorkspaceRoot(), "Build", "DATABASE.db")
        if not DB_PATH.exists(): # TODO: Add cli arg to force parsing
            try:
                logging.log(SUB_SECTION , "Running stuart_parse")
                RunCmd("stuart_parse", '', workingdir = PLATFORMBUILD_DIR, logging_level=logging.DEBUG, raise_exception_on_nonzero=True)
            except Exception:
                logging.error("Failed to run stuart_parse. Review Build/PARSE_LOG.txt")
                return -1
        else:
            logging.warning("Skipping Parse as DATABASE.db already exists. Force a re-parse with --ForceParse")

        # Verify that any file that is both used by the platform, and tested by a test, is in the dsc.
        logging.log(SECTION, "Verify Host Based Tests are Up to Date")
        dscp = DscParser()
        dscp.SetEdk2Path(self.edk2path).SetInputVars(self.env.GetAllBuildKeyValues() | self.env.GetAllNonBuildKeyValues())
        dscp.ParseFile(self.env.GetValue("ACTIVE_PLATFORM"))

        used_tests = set([component for component, _, _ in dscp.Components])
        must_use_tests = set([module for module, in Edk2DB(DB_PATH).connection.execute(TEST_QUERY).fetchall()])
        unused_tests = must_use_tests - used_tests
        if len(unused_tests) > 0:
            logging.error("The following host based unit tests test files used by the "
                          "platform, but are not in the host based unit test dsc:")
            logging.error("\n  ".join(unused_tests))
            return -1
        logging.info("Host Based Tests are up to date.")

        return 0

    def PlatformFlashImage(self):
        reporttypes = self.env.GetValue("REPORTTYPES").split(",")
        logging.log(SECTION, "Generating Requested Code Coverage Reports")
        logging.info(f'Report Types: {",".join(reporttypes)}')

        # Organize existing report by platform
        coverage_file = str(Path(self.GetWorkspaceRoot(), "Build", "coverage.xml"))
        params = "coverage"
        params += f' {coverage_file}'
        params += f' -ws {self.GetWorkspaceRoot()}'
        params += ' --by-platform'
        params += f' -d {PLATFORM_DSC}'
        params += f' -o {coverage_file}'
        try:
            RunCmd("stuart_report", params, logging_level=logging.DEBUG, raise_exception_on_nonzero=True)
        except Exception:
            logging.error("stuart_report Failed to generate a report.")
            return -1
        
        # Generate the requested reports
        out_cov_dir = Path(self.env.GetValue("BUILD_OUTPUT_BASE"), "Coverage")
        params = f'-reports:"{coverage_file}"'
        params += f' -targetdir:"{str(out_cov_dir)}"'
        params += f' -reporttypes:{";".join(reporttypes)}'
        try:
           RunCmd("reportgenerator", params, raise_exception_on_nonzero=True, logging_level=logging.DEBUG)
        except Exception:
            logging.error("reportgenerator Failed to generate a report.")
            return -1
        
        # Clean up the raw coverage file
        Path(coverage_file).unlink()
        return 0

    def _verify_code_cov_tools(self) -> bool:
        return True
        try:
            RunCmd("reportgenerator", "-h", logging_level=logging.DEBUG, raise_exception_on_nonzero=True)
        except Exception:
            logging.error("You do not have reportgenerator installed, but are requesting a report be generated.")
            logging.error("You can install with `dotnet tool install -g dotnet-reportgenerator-globaltool`")
            return False
        
        if sys.platform.startswith("win"):
            try:
                RunCmd("OpenCppCoverage", "", logging_level=logging.DEBUG, raise_exception_on_nonzero=True)
            except Exception:
                logging.error("You do not have OpenCppCoverage installed, but are requesting a report be generated.")
                return False
        else:
            try:
                RunCmd("lcov", "--version", logging_level=logging.DEBUG, raise_exception_on_nonzero=True)
            except Exception:
                logging.error("You do not have lcov installed, but are requesting a report be generated.")
                return False
            try:
                RunCmd("lcov_cobertura", "--version", logging_level=logging.DEBUG, raise_exception_on_nonzero=True)
            except Exception:
                logging.error("You do not have lcov_cobertura installed, but are requesting a report be generated.")
                return False
        return True