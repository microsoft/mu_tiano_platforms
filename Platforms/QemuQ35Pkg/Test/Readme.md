# QemuQ35Pkg Platform Testing

`PlatformTest.py` is used to to execute host based unit tests for tests associated with a platform. In the pre-build phase, host-based unit tests are validated for currency using a straightforward method. This method ensures that any host-based unit test sharing a source file with an INF (Library or Driver) utilized by the platform is also included in the platform's list of host-based unit tests to compile and execute. If a host based test is found to be missing, it will stop the build. It should be noted that this is not perfect, as tests that are testing protocol interfaces and other miscellaneous scenarios won't be caught as the source files are not present in the host based unit test's INF.

This is different than the `stuart_ci_build` process for compiling and executing host based unit tests as they are bundled by package where as a platform's host based unit tests can contain tests from any package. There are also differences in how code coverage results are created, which will be looked at next.

## Code Coverage

Code coverage can be enabled when building with `PlatformTest.py` by adding `CODE_COVERAGE=TRUE` to your command line. This does require additional tools to be installed, as noted below:

* Windows Prerequisite

  1. OpenCppCoverage: Download and install <https://github.com/OpenCppCoverage/OpenCppCoverage/releases>
  2. pygount (if using the --full comand): pip install pygount

* Linux Prerequisite

  1. lcov: sudo apt-get install -y lcov
  2. lcov_cobertura: pip install lcov_cobertura
  3. pygount (if using the --full comand): pip install pygount

  Using this command, 
