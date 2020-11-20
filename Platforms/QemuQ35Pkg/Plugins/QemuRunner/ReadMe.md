# QemuRunner_plug_in

The QEMU runner plugin supports easy execution of the QEMU emulator running the locally compiled
QemuQ35Pkg firmware.  This runner also supports easy local and server execution of UEFI shell based tests.
It can automatically collect compiled UEFI shell based unit tests, mount a VHD or map a folder as a drive,
and then parse the results once QEMU has finished.

## Configuration

The plugin has numerous configuration options to support the Q35 Platform.  These can be set
when calling `stuart_build` or `platform_build` by adding `<name>=<value>` to the command line.

**Example** setting up unit test to run automatically

```bash
stuart_build -c Platforms/QemuQ35Pkg/PlatformBuild.py MAKE_STARTUP_NSH=TRUE RUN_UNIT_TESTS=TRUE
```

### QEMU_HEADLESS

Boolean string value to indicate if QEMU should be configured to run headless/no graphics.
By default graphics will be used but in some server/remote scenarios headless is required.

**TRUE**:   configure QEMU to run headless or with no graphics  
**FALSE**:  configure QEMU for local graphics (default)

### MAKE_STARTUP_NSH

Boolean string value to indicate the plugin should generate a `startup.nsh` file and copy
it to a folder that will be mapped as a virtual drive for QEMU.  This startup.nsh is a special
file that executes when the UEFI shell loads.  See UEFI shell specification for more details. By default
the startup.nsh file will contain a single command to shutdown the system.  This is useful for determining
if the local compiled firmware is capable of booting to the UEFI shell without a software assert, system
exception, or catastrophic firmware failure.  

**TRUE**:   make `startup.nsh` file  
**FALSE**:  do not make `startup.nsh` file. (default)

### RUN_UNIT_TESTS

Boolean string value to indicate the plugin should attempt to locate all compiled UEFI shell based
unit tests, copy them to the virtual drive folder, and add them to the `startup.nsh`.  For this to have any
effect it must be combined with `MAKE_STARTUP_NSH=TRUE`.  After QEMU is done executing
the plugin will attempt to parse the XML based results and display the results.

**TRUE**:   find, execute, and evaluate UEFI shell unit tests  
**FALSE**:  do not (default)

### STARTUP_GLOB_CSV

String comma separated value to configure the plugin on how to identify a UEFI shell based unit test.  By default
the plugin will look for `*Test*.efi` in the build output directory.  This can be changed by setting this parameter
to a CSV of glob patterns.

Example: `STARTUP_GLOB_CSV=MyTestOne.efi,*UefiShellApp.efi`

### STARTUP_NSH_DIRTY

Boolean string value to control plugin creation of virtual drive folder and emptying the folder before copying
contents.  When running automated tests multiple times they may write some state to the virtual drive.  This state
may change their execution.  Sometimes that is desired and this option allows the plugin to copy new files to the virtual
drive but not delete files.  

**TRUE**:   don't delete all drive content before copying new content  
**FALSE**:  delete all drive contents before copying new content (default)
