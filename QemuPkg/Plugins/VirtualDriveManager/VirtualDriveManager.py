##
# This plugin loads a virtual harddrive image with tests.
#
# Copyright (c) Microsoft Corporation
# SPDX-License-Identifier: BSD-2-Clause-Patent
##

import errno
import logging
import string
import shutil
import tempfile
import os
import xml.etree.ElementTree

from os import PathLike
from pathlib import Path

from edk2toolext.environment.plugintypes.uefi_helper_plugin import IUefiHelperPlugin
from edk2toolext.environment import shell_environment
from edk2toollib.utility_functions import RunCmd
from html import unescape


logger = logging.getLogger(__name__)


class StartupScript:
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

    def write_out(self, host_file_path, shutdown):

        with open(host_file_path, "w") as nsh:
            if self._use_fs_finder:
                this_file = os.path.basename(host_file_path)
                nsh.write(self.FS_FINDER_SCRIPT.format(first_file=this_file))

            for l in self._lines:
                nsh.write(l + "\n")

            if shutdown:
                nsh.write("reset -s\n")

    def add_line(self, line: str):
        self._lines.append(line.rstrip())
        self._use_fs_finder = True


class VirtualDrive:
    """A base class for managing virtual drives.

    Attributes:
        drive_path (Path): the path to the drive
    """
    def __init__(self, path: PathLike):
        """Initializes the virtual drive."""
        self.drive_path = Path(path)

    def exists(self) -> bool:
        """Returns if the Virtual drive exists at `drive_path`."""
        return self.drive_path.exists()

    def wipe(self, size: int = 60):
        """Deletes the virtual drive and creates an empty one at the same location."""
        self.delete()
        self.make_drive(size)
    
    def delete(self):
        """Deletes the virtual drive."""
        self.drive_path.unlink(missing_ok=True)

    def add_startup_script(self, lines: list[str] = [], auto_shutdown = True):
        """Adds a startup script that executes on boot.

        Args:
            lines (list[str]): A list of lines to execute on startup
            auto_shutdown (Boolean): Whether or not to add a line that shuts down system after all lines have excuted.

        !!! note
            `lines`  does not need to be a str, but it must be convertable to a str with `str()`
        """
        nsh = StartupScript()
        for line in lines:
            nsh.add_line(str(line))

        nsh_path = self.drive_path.parent / "startup.nsh"
        nsh.write_out(nsh_path, auto_shutdown)
        self.add_file(nsh_path)

    def add_files(self, filepaths: list[str]):
        """Adds files to the root directory of the virtual drive."""
        for filepath in filepaths:
            self.add_file(filepath)

    def add_file(self, file: PathLike):
        """Adds a file to the root directory of the virtual drive."""
        raise NotImplementedError

    def make_drive(self, size: int = 60):
        """Creates a virtual drive at self.drive_path."""
        raise NotImplementedError

    def get_file(self, virtual_path: PathLike, local_path: PathLike):
        """Retrieves a file from the virtual drive.

        Args:
            virtual_path (PathLike): Path on the virtual drive to the file
            local_path (PathLike): Path to place the file.
        """
        raise NotImplementedError

    def get_file_contents(self, virtual_path: PathLike, local_path: PathLike = None) -> str:
        """Gets a contents from a file from the virtual drive. Optionally save the file too.

        Args:
            virtual_path (PathLike): The path to the file on the virtual drive
            local_path (PathLike): The path to save the file to

        Raises:
            (RuntimeError): Failed to get the filepath
        """
        raise NotImplementedError


class LinuxVirtualDrive(VirtualDrive):
    def __init__(self, path: PathLike):
        super().__init__(path)
        self.drive_letter = self._find_unused_drive_letter()

        conf_path = os.path.join(os.path.dirname(self.drive_path), "mtool.conf")
        if os.path.exists(conf_path):
            shell_environment.GetEnvironment().set_shell_var("MTOOLSRC", conf_path)
        elif os.path.exists(self.drive_path):
            logger.error("VirtualDrive.img exists from a previous build but "
                         "mtool.conf is not in the same directory. Add "
                         f"mtool.conf or delete {self.drive_path} to recreate "
                         "the virtual drive and the corresponding mtool.conf "
                         "file.")
            raise FileNotFoundError(
                errno.ENOENT, os.strerror(errno.ENOENT), conf_path)

    def make_drive(self, size: int = 60):
        """Creates a virtual hard drive

        Args:
            size (int | Optional): The size of the hard drive in MB

        Raises:
        (RuntimeError): The drive could not be created
        """
        # Create an image
        cmd = "dd"
        args = f"if=/dev/zero of={self.drive_path} bs=1M count={size}"
        result = RunCmd(cmd, args)
        if result != 0:
            e = f"[{cmd} {args}] Result: {result}"
            logger.error("Drive could not be created.")
            logger.error(e)
            raise RuntimeError(e)

        # Format the image as FAT32
        cmd = self._locate_cmd("mkfs.vfat")
        if not cmd:
            e = f"Could not locate executable `mkfs.vfat` on the $PATH."
            logging.error(e)
            raise RuntimeError(e)

        args = f"{self.drive_path}"
        result = RunCmd(cmd, args)
        if result != 0:
            e = f"[{cmd} {args}] Result: {result}"
            logger.error("Failed to format drive")
            logger.error(e)
            raise RuntimeError(e)

        rc = 0
        # Create an mtools config file to virtually map the image to a drive letter
        conf_path = os.path.join(os.path.dirname(self.drive_path), "mtool.conf")
        if os.path.exists(conf_path):
            # This should be repopulated per build
            rc |= RunCmd("rm", conf_path)
        rc |= RunCmd("echo", "mtools_skip_check=1 > ~/.mtoolsrc")
        rc |= RunCmd("echo", f"drive+ {self.drive_letter}: >> {conf_path}")
        rc |= RunCmd("echo", f"\"  file=\\\"{self.drive_path}\\\" exclusive\" >> {conf_path}")
        shell_environment.GetEnvironment().set_shell_var ("MTOOLSRC", conf_path)
        
        if rc != 0:
            self.delete()
            e = f"Failed to map image to drive letter. Review above command errors."
            logger.error(e)
            raise RuntimeError(e)

    def add_file(self, filepath: PathLike):
        """Adds a file to the virtual drive."""
        cmd = "mcopy"
        args = f"-D overwrite -i {str(self.drive_path)} {filepath} {self.drive_letter}:"
        result = RunCmd(cmd, args)
        if result != 0:
            e = f"[{cmd} {args}] Result: {result}"
            logger.error(f"Failed to insert {filepath} into drive.")
            logger.error(e)
            raise RuntimeError(e)

    def get_file(self, virtual_path: PathLike, local_path: PathLike):
        """Gets a file from the virtual drive.

        Args:
            virtual_path (PathLike): The path to the file on the virtual drive
            local_path (PathLike): The path to save the file to

        Raises:
            (RuntimeError): Failed to get the filepath
        """
        cmd = "mcopy"
        full_path = os.path.join(self.drive_letter + ":", virtual_path)
        args = f"-n -i {self.drive_path} {full_path} {local_path}"
        result = RunCmd(cmd, args)
        if result != 0:
            e = f"[{cmd} {args}] Result: {result}"
            logger.error(f"Failed to get {virtual_path} from drive.")
            logger.error(e)
            raise RuntimeError(e)

    def get_file_contents(self, virtual_path: PathLike, local_path: PathLike = None):
        """Gets a contents from a file from the virtual drive. Optionally save the file too.

        Args:
            virtual_path (PathLike): The path to the file on the virtual drive
            local_path (PathLike): The path to save the file to

        Raises:
            (RuntimeError): Failed to get the filepath
        """
        if local_path is None:
            local_path = tempfile.mktemp()
        self.get_file(virtual_path, local_path)

        with open(local_path, "rb") as f:
            return f.read()

    def _find_unused_drive_letter(self):
        for drive_letter in string.ascii_lowercase:
            cmd = "grep"
            args = f"-i '/mnt/{drive_letter} ' /etc/mtab"
            result = RunCmd(cmd, args)

            # per man grep, ret 1 is no lines matched, so we can return
            if result == 1:
                return drive_letter

            # per man grep, ret 0 is lines were matched
            elif result == 0:
                continue

            else:
                e = f"[{cmd} {args}] Result: {result}"
                logger.error("Failed to check if drive letter is in use.")
                logger.error(e)
                raise RuntimeError(e)

        raise ValueError("No unused drive letters available")

    def _locate_cmd(self, cmd: str) -> str | None:
        """Locates a command on a linux file system using `which`."""  
        # 1. Search for the command on the $PATH
        path = shutil.which(cmd, mode = os.X_OK)
        if path:
            return path
        
        # 2. Search for the command in /sbin in case /sbin is not on the $PATH
        path = shutil.which(cmd, mode = os.X_OK, path = "/sbin")
        if path:
            return path
        
        # 3. Search for the command in /usr/sbin in case /usr/sbin is not on the $PATH
        path = shutil.which(cmd, mode = os.X_OK, path = "/usr/sbin")
        if path:
            return path
        
        # 4. We failed to find it in our normal search paths
        return None

class WindowsVirtualDrive(VirtualDrive):
    def __init__(self, path: PathLike):
        super().__init__(path)
        self.drive_path = Path(path)

    def make_drive(self, size: int = 60):
        """Creates a virtual hard drive

        Args:
            size (int | Optional): The size of the hard drive in MB

        Raises:
            (RuntimeError): The drive could not be created
        """
        # Create an image
        cmd = "VHDCreate"
        args = f"-sz {size}MB {self.drive_path}"
        result = RunCmd(cmd, args)
        if result != 0:
            e = f"[{cmd} {args}] Result: {result}"
            logger.error("Drive could not be created.")
            logger.error(e)
            raise RuntimeError(e)

        # Format the image as FAT32
        cmd = "DiskFormat"
        args = f"-ft fat -ptt bios {self.drive_path}"
        result = RunCmd(cmd, args)
        if result != 0:
            e = f"[{cmd} {args}] Result: {result}"
            logger.error("Failed to format drive")
            logger.error(e)
            raise RuntimeError(e)

    def add_file(self, filepath: PathLike):
        """Adds a file to the virtual drive."""
        filename = Path(filepath).name

        cmd = "FileInsert"
        args = f"{filepath} {filename} {str(self.drive_path)}"
        result = RunCmd(cmd, args)
        if result != 0:
            e = f"[{cmd} {args}] Result: {result}"
            logger.error(f"Failed to insert {filepath} into drive.")
            logger.error(e)
            raise RuntimeError(e)

    def get_file(self, virtual_path: PathLike, local_path: PathLike):
        """Gets a file from the virtual drive.

        Args:
            virtual_path (PathLike): The path to the file on the virtual drive
            local_path (PathLike): The path to save the file to

        Raises:
            (RuntimeError): Failed to get the filepath
        """
        cmd = "FileExtract"
        args = f"{virtual_path} {local_path} {self.drive_path}"
        result = RunCmd(cmd, args)
        if result != 0:
            e = f"[{cmd} {args}] Result: {result}"
            logging.error(f"Failed to get {virtual_path} from drive.")
            logging.error(e)
            raise RuntimeError(e)

    def get_file_contents(self, virtual_path: PathLike, local_path: PathLike = None):
        """Gets a contents from a file from the virtual drive. Optionally save the file too.

        Args:
            virtual_path (PathLike): The path to the file on the virtual drive
            local_path (PathLike): The path to save the file to

        Raises:
            (RuntimeError): Failed to get the filepath
        """
        if local_path is None:
            local_path = tempfile.mktemp()

        self.get_file(virtual_path, local_path)

        with open(local_path, "rb") as f:
            return f.read()


class VirtualDriveManager(IUefiHelperPlugin):
    def RegisterHelpers(self, obj):
        fp = str(Path(__file__).absolute())
        obj.Register("get_virtual_drive", VirtualDriveManager.get_virtual_drive, fp)
        obj.Register("add_tests", VirtualDriveManager.add_tests, fp)
        obj.Register("report_results", VirtualDriveManager.report_results, fp)
        obj.Register("generate_paging_audit", VirtualDriveManager.generate_paging_audit, fp)
        return 0

    @staticmethod
    def get_virtual_drive(path: PathLike):
        if os.name == 'nt':
            return WindowsVirtualDrive(path)
        return LinuxVirtualDrive(path)

    @staticmethod
    def add_tests(drive: VirtualDrive, test_list: list[str], auto_run = True, auto_shutdown = True, paging_audit = False):
        """Adds tests to the virtual drive and optionally adds them to the startup script.

        !!! note
            Typically, to run a test, one must only run the test efi, however to automate this process, we have added
            slightly more complex logic to the startup.nsh to ensure the tests will run and provide a fresh set of
            results each time tests are added to the it.

            1. We conditionally run a test based off of the presence of a <TestName>_JUNIT.XML. When this file is
               present, it means that the test has completely finished and the results have been recorded. Once the
               tests have finished, we rename them to _JUNIT_RESULT.XML so that the presence of these files will not
               stop tests from running again.

            2. After all tests have finished, we delete two types of files. The first is a previous run's
               _JUNIT_RESULT.XML. This is necessary as the mv cannot overwrite a file. The second is that we delete
               a test's _Cache.dat file. This is important because if we do not delete this file, the efi will execute,
               but the test will not run. This is because a test uses the _Cache.dat file to see the current status
               of the test (important for tests that require restarts)

            3. We finally rename the current test results to _JUNIT_RESULT.XML to reset test progress and also allow
               for test results to be read after Qemu has shut down.
        Args:
            drive (VirtualDrive): The virtual drive to add the tests to.
            auto_run (Boolean): Whether or not to run tests automatically.
            auto_shutdown (Boolean): Whether or not to shutdown after tests have completed.
        """
        drive.add_files(test_list)
        tests = []

        if auto_run:
            # Execute all tests
            tests.append("# 1. We conditionally run a test based off of the presence of a <TestName>_JUNIT.XML. When this file is")
            tests.append("#    present, it means that the test has completely finished and the results have been recorded. Once the")
            tests.append("#    tests have finished, we rename them to _JUNIT_RESULT.XML so that the presence of these files will not")
            tests.append("#    stop tests from running again.")
            tests.append("# 2. After all tests have finished, we delete two types of files. The first is a previous run's")
            tests.append("#    _JUNIT_RESULT.XML. This is necessary as the mv cannot overwrite a file. The second is that we delete")
            tests.append("#    a test's _Cache.dat file. This is important because if we do not delete this file, the efi will execute,")
            tests.append("#    but the test will not run. This is because a test uses the _Cache.dat file to see the current status")
            tests.append("#    of the test (important for tests that require restarts)")
            tests.append("# 3. We finally rename the current test results to _JUNIT_RESULT.XML to reset test progress and also allow")
            tests.append("#    for test results to be read after Qemu has shut down.")
            for test in test_list:
                tests.append(f"if not exist {test.stem}_JUNIT.XML then")
                tests.append(f"    {test.name}")
                tests.append("endif")

            # Tests have finished, no more restarts
            # Remove any old test results and reset test status
            tests.append("rm *_JUNIT_RESULT.XML")
            tests.append("rm *_Cache.dat")

            # Rename test results to what we expect
            for test in test_list:
                tests.append(f"if exist {test.stem}_JUNIT.XML then")
                tests.append(f"    mv {test.stem}_JUNIT.XML {test.stem}_JUNIT_RESULT.XML")
                tests.append("endif")

            if paging_audit:
                tests.append("DxePagingAuditTestApp.efi -d")

        drive.add_startup_script(tests, auto_shutdown = auto_shutdown)

    @staticmethod
    def report_results(drive: VirtualDrive, test_list: list[str], result_output_dir: Path) -> list[(str, str)]:
        """Prints test results to the terminal and returns the number of failed tests."""
        result_output_dir.mkdir(exist_ok=True)

        failure_count = 0
        for test in test_list:
            result_file = test.stem + "_JUNIT_RESULT.XML"
            local_file_path = result_output_dir / result_file
            try:
                data = drive.get_file_contents(result_file, local_file_path)
            except:
                logging.error(f"unit test ({test}) produced no result file.")
                failure_count += 1
                continue

            try:
                # Parse the xml data to gather all results
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
                                failure_count += 1
                        logging.log( level, caseresult)

            except Exception as ex:
                logging.error("Exception trying to read xml." + str(ex))
                failure_count += 1
        return failure_count

    @staticmethod
    def generate_paging_audit(drive: VirtualDrive, report_output_dir: Path, version: str, platform: str):
        paging_audit_data_files = ["1G.dat", "2M.dat", "4K.dat", "MAT.dat",
                                   "GuardPage.dat", "MemoryInfoDatabase.dat", "PlatformInfo.dat"]
        paging_audit_generator_path = os.path.join("Common", "MU", "UefiTestingPkg", "AuditTests",
                                                   "PagingAudit", "Windows", "PagingReportGenerator.py")
        report_output_dir.mkdir(exist_ok=True)
        for file in paging_audit_data_files:
            drive.get_file(file, os.path.join(report_output_dir, file))
        output_audit = os.path.join(report_output_dir, "pagingaudit.html")
        output_debug = os.path.join(report_output_dir, "pagingauditdebug.txt")
        cmd = "python"
        args = f"{paging_audit_generator_path} -i {report_output_dir} -o {output_audit} \
-p {platform} --debug -l {output_debug} --PlatformVersion {version}"
        result = RunCmd(cmd, args)
        if result != 0:
            e = f"[{cmd} {args}] Result: {result}"
            logger.error("Paging audit could not be created.")
            logger.error(e)