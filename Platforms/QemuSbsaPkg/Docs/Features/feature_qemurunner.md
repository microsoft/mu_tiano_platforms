# QemuRunner_plug_in

The QEMU runner plugin supports easy execution of the QEMU emulator running the locally compiled
QemuSbsaPkg firmware.  This runner also supports easy local and server execution of UEFI shell based tests.
It can automatically collect compiled UEFI shell based unit tests, mount a VHD or map a folder as a drive,
and then parse the results once QEMU has finished.

## Configuration

The plugin has numerous configuration options to support the SBSA Platform.  These can be set
when calling `stuart_build` or `platform_build` by adding `<name>=<value>` to the command line.

**Example** setting up unit test to run automatically

```bash
stuart_build -c Platforms/QemuSbsaPkg/PlatformBuild.py SHUTDOWN_AFTER_RUN=TRUE RUN_TESTS=TRUE
```

### QEMU_HEADLESS

Boolean string value to indicate if QEMU should be configured to run headless/no graphics.
By default graphics will be used but in some server/remote scenarios headless is required.

**TRUE**:   configure QEMU to run headless or with no graphics  
**FALSE**:  configure QEMU for local graphics (default)

### TEST_REGEX

Comma separated regular expressions to configure the plugin on how to identify a UEFI shell based
unit test. If one is provided and the user is on a Windows OS, all tests found with the regular
expressions will be added to the virtual drive

Example: `TEST_REGEX=MyTestOne.efi,*UefiShellApp.efi`

### RUN_TESTS

Boolean string value to indicate the plugin should write all shell-based unit tests located with
`TEST_REGEX` to `startup.nsh`.This startup.nsh is a special file that executes when the UEFI shell
loads. See UEFI shell specification for more details. Unless `SHUTDOWN_AFTER_RUN=FALSE` is also passed,
QEMU will shutdown after executing to parse and display the XML based results.

**TRUE**:   find, execute, and evaluate UEFI shell unit tests  
**FALSE**:  do not (default)

### SHUTDOWN_AFTER_RUN

Boolean string value to indicate that QEMU should be shutdown once it has finished running. The
system is finished running when it has booted to shell or all unit tests specified by `TEST_REGEX`
and added to the `startup.nsh` script with `RUN_TESTS=TRUE` have finished execution.

**TRUE**:   The system will automaticaly shutdown after booting to shell or running all unit tests  
**FALSE**:  The system will not automatically shutdown (default)

### EMPTY_DRIVE

Boolean string value to control plugin creation of virtual drive folder and emptying the folder before copying
contents. When running automated tests multiple times they may write some state to the virtual drive which may
change their execution. Sometimes that is desired and this option allows the plugin to copy new files to
the virtual drive but not delete files.

**TRUE**:   delete all drive contents before copying new content
**FALSE**:  don't delete all drive content before copying new content (default)
