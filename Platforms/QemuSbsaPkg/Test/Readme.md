# QemuQ35Pkg Platform Testing

`PlatformTest.py` is used to to execute host based unit tests for tests associated with a platform. In the pre-build
phase, host-based unit tests are validated for currency using a straightforward method. This method ensures that any
host-based unit test sharing a source file with an INF (Library or Driver) utilized by the platform is also included
in the platform's list of host-based unit tests to compile and execute. If a host based test is found to be missing,
it will stop the build. It should be noted that this is not perfect, as there are tests that test protocol interfaces
or other miscellaneous scenarios won't be caught as source files are not present in the host based unit test's INF.

This is different than the `stuart_ci_build` process for compiling and executing host based unit tests as they are
bundled by package where as a platform's host based unit tests can contain tests from any package. There are also
differences in how code coverage results are created, which will be looked at next.

## Code Coverage

Code coverage can be enabled when building with `PlatformTest.py` by adding `CODE_COVERAGE=TRUE` to your command line.
This does require additional tools to be installed, which is verified as a pre-build step. These tools are noted below:

* Windows Prerequisite

  1. OpenCppCoverage: Download and install <https://github.com/OpenCppCoverage/OpenCppCoverage/releases>
  2. pygount (if using the --full comand): pip install pygount

* Linux Prerequisite

  1. lcov: sudo apt-get install -y lcov
  2. lcov_cobertura: pip install lcov_cobertura
  3. pygount (if using the --full comand): pip install pygount

With `CODE_COVERAGE=TRUE`, a coverage report will be geenrated at
`Build/QemuSbsaPkg/HostTest/NOOPT_<TOOL_CHAIN>/QemuSbsaPkg_coverage.xml`. Other optional config knobs are
`CC_FLATTEN=TRUE`, which removes duplicate source code coverage information, which happens due to some INFs re-using
source files and `CC_FULL=TRUE`, which will create xml data for files used by the platform that does not have existing
coverage information. This provides a more accurate view of overall code coverage for a platform.

Otherwise, there are a plethora of open source tools for generating reports from a Cobertura file, which is why it was
selected as the output file format. Tools such as pycobertura (`pip install pycobertura`) and
[reportgenerator](https://www.nuget.org/packages/dotnet-reportgenerator-globaltool) can be utilized to generate
different report types, such as local html reports. VSCode Extensions such as
[Coverage Gutters](https://marketplace.visualstudio.com/items?itemName=ryanluker.vscode-coverage-gutters) can highlight
coverage results directly in the file, and cloud tools such as [CodeCov](https://about.codecov.io/) can consume
cobertura files to provide PR checks and general code coverage statistics for the repository.

If you have `reportgenerator` installed, you can additionally set `REPORTTYPES` to any report type that
`reportgenerator` can generate, and those reports will be generated at
`Build/QemuSbsaPkg/HostTest/NOOPT_<TOOL_CHAIN>/Coverage/*`. This parameter supports a comma separated list such as
`REPORTTYPES=HtmlSummary,JsonSummary`.
