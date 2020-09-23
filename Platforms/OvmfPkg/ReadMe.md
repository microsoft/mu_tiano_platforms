# OvmfPkg

This package supports building the Tianocore edk2 [OvmfPkg](https://github.com/tianocore/edk2/tree/master/OvmfPkg) with
[Project Mu](https://microsoft.github.io/mu/).

The only changes in this package from the upstream should be related to build and repository layout differences.  This particular
package/platform does not showcase or enable the Project Mu feature set but it does serve as a convenient dependency
and, in some cases, code delta for the other platform packages in this repository that do.

## More Details

Building with Stuart - See [OvmfPkg/PlatformCI/ReadMe.md](PlatformCI/ReadMe.md)

For other details about OVMF and QEMU see the Edk2 maintained readme at OvmfPkg/README.
