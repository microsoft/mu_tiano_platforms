# Platform Package Testing

`PlatformTest.py` is used to compile and execute host based unit tests for tests associated with a platform. In the pre-build phase, host-based unit tests are validated for currency using a straightforward method. This method ensures that any host-based unit test sharing a source file with an INF (Library or Driver) utilized by the platform is also included in the platform's list of host-based unit tests to compile and execute. If a host based unit test is found to be missing, it will stop the build. It should be noted that this is not perfect, as there are tests for protocol interfaces or other miscellaneous scenarios that won't be caught because the source files are not present in the host based unit test's INF.

This method of compiling and running host-based unit tests is different than the typical method, which is to use `stuart_ci_build` with the target (`-t`) of `NOOPT`. This is to support the pre-build step of validating that the host-based unit test DSC is up to date, and to allow for differences in how code coverage results are created, which will be looked at next.

## Code Coverage

By default, code coverage is disabled. It is enabled via the command line by adding `CODE_COVERAGE=TRUE` when building
with `PlatformTest.py`. Code coverage does require additional tools to be installed, which are verified in a pre-build
step. The required tools are noted below, per operating system:

* Windows Prerequisite

  1. OpenCppCoverage: Download and install <https://github.com/OpenCppCoverage/OpenCppCoverage/releases>
  2. pygount: (if using the --full command) pip install pygount

* Linux Prerequisite

  1. lcov: sudo apt-get install -y lcov
  2. lcov_cobertura: pip install lcov_cobertura
  3. pygount: (if using the --full command) pip install pygount

A coverage report will be generated at `Build/<PKG_NAME>/HostTest/NOOPT_<TOOL_CHAIN>/<PKG_NAME>_coverage.xml`. Other
optional config knobs are `CC_FLATTEN=TRUE`, which removes duplicate source file coverage information, which happens
when multiple INFs refer to the same source files. The second is `CC_FULL=TRUE`, which will create xml data for source
files used by the platform that do not have any existing coverage information. This provides a more accurate view of
overall code coverage for a platform.

There are a plethora of open source tools for generating reports from cobertura xml files, which is why it was selected
as the output file format. Tools such as pycobertura (`pip install pycobertura`) and
[reportgenerator](https://www.nuget.org/packages/dotnet-reportgenerator-globaltool) can be utilized to generate
different report types, such as html reports. VSCode extensions such as
[Coverage Gutters](https://marketplace.visualstudio.com/items?itemName=ryanluker.vscode-coverage-gutters) can highlight
coverage results directly in source files, and cloud tools such as [CodeCov](https://about.codecov.io/) can consume
cobertura files to provide PR checks and general code coverage statistics for the repository.

If you have `reportgenerator` installed, you can additionally set `REPORTTYPES` to any report type that
`reportgenerator` can generate, and those reports will be generated at
`Build/QemuSbsaPkg/HostTest/NOOPT_<TOOL_CHAIN>/Coverage/*`. This parameter supports a comma separated list such as
`REPORTTYPES=HtmlSummary,JsonSummary`.
