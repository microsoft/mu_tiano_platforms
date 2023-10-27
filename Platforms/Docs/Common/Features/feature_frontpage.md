# Front Page

The Project Mu graphical front page provides access to system information, boot option configuration, and
[DFCI](https://github.com/microsoft/mu_feature_dfci) configuration settings.

For more information about front page, refer to its documentation -
[OemPkg - Front Page](https://github.com/microsoft/mu_oem_sample/blob/HEAD/Docs/OemPkg.md).

![Front page in Q35](mu_frontpage.gif)

## Entering Front Page

Two build variables are available that influence how front page is built and loaded.

1. `BLD_*_GUI_FRONT_PAGE=TRUE` - Builds the front page application shown here instead of alternative non-GUI
   applications that act as the front page. The default value for this variable is `FALSE`, therefore it must be set
   to `TRUE`.
2. `BOOT_TO_FRONT_PAGE=TRUE` - Simulates pressing the Vol+ button which indicates that the boot should prioritize the
   front page boot option. If this is not done, but (1) is set to `TRUE`, this front page can still be entered but it
   is not loaded automatically. Instead, the boot will likely boot to the EFI shell and then exiting the shell (with
   the `exit` command), will switch to this front page.

   Once in front page, the boot option order can be configured as desired. This front page's boot option entry is named
   "Mu UEFI UI Front Page".

   The default value for this variable is `FALSE`.
