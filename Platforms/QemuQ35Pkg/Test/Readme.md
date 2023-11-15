# QemuQ35Pkg Platform Testing

`PlatformTest.py` is used to to execute host based unit tests for tests associated with a platform. In the pre-build phase, host-based unit tests are validated for currency using a straightforward method. This method ensures that any host-based unit test sharing a source file with an INF (Library or Driver) utilized by the platform is also included in the platform's list of host-based unit tests to compile and execute. If a host based test is found to be missing, it will stop the build. It should be noted that this is not perfect, as tests that are testing protocol interfaces and other miscellaneous scenarios won't be caught as the source files are not present in the host based unit test's INF.

This is different than the `stuart_ci_build` process for compiling and executing host based unit tests as they are bundled by package where as a platform's host based unit tests can contain tests from any package. There are also differences in how code coverage results are created, which will be looked at next.

## Code Coverage


