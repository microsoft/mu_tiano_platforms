# QemuRunner_plug_in

Q35 customized helper plugin to provide functions for running Q35 firmware in QEMU and supporting
additional features like shell based unit tests

## Configuration

The plugin has numerous configuration options to support the Q35 Platform.

```py
# STARTUP_NSH_DIRTY:
#                     True:  don't empty folder before copying.
#                     False: empty folder before copying new contents
# QEMU_HEADLESS:
#                     True:  configure QEMU to run headless (no graphics)
#                     False: configure QEMU for local graphics
#
# MAKE_STARTUP_NSH:
#                     True: Create a startup nsh file with all unit tests
#                     False: don't create startup.nsh file
#
# RUN_UNIT_TESTS:
#                     True: if combined with MAKE_STARTUP_NSH will add all unit tests
#                           to startup nsh and copy efi files to virtual drive
#                     False: Don't copy unit tests to virtual drive and don't add to nsh
#
#
# STARTUP_GLOB_CSV:
#                     CSV of glob patterns to use to add to startup nsh.  Default is *Test*.efi
#
# BUILD_OUTPUT_BASE: Output directory for this build
```
