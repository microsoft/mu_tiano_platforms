# CodeQL

## Overview

CodeQL is open source and free for open-source projects. It is maintained by GitHub and naturally has excellent
integration with GitHub projects. CodeQL uses a semantic code analysis engine to discover vulnerabilities in a
number of programming languages (both compiled and interpreted).

[General CodeQL Information](https://codeql.github.com/)

Project Mu (and TianoCore) use CodeQL C/C++ queries to find common programming errors and security vulnerabilities in
firmware code. This platform leverages the CodeQL build plugin from Mu Basecore that makes it very easy to run CodeQL
against this platform. You simply use provide the `--codeql` argument in your normal `stuart_update` and `stuart_build`
commands.

## CodeQL Command-Line Interface (CLI)

Because of CodeQL's integration with GitHub, it is often run in projects hosted on GitHub via an officially supported
GitHub Action ([codeql-action](https://github.com/github/codeql-action)).

However, a [CodeQL CLI application](https://codeql.github.com/docs/codeql-cli/) is also available that provides a
command-line interface to CodeQL. This facilitates a local developer workflow by using the CLI application to
perform two main tasks:

1. Generate a CodeQL database
2. Analyze the CodeQL database

There's ample documentation written on [creating CodeQL databases](https://codeql.github.com/docs/codeql-cli/creating-codeql-databases/)
and [analyzing CodeQL databases](https://codeql.github.com/docs/codeql-cli/analyzing-databases-with-the-codeql-cli/).

Our unique firmware build environment poses several challenges and further integrating the CLI with the stuart tool
set can be daunting for those unfamiliar with stuart's internals.

Therefore, Project Mu and, by extension, this platform, use a set of CodeQL plugins from Mu Basecore to simplify
CodeQL usage.

## CodeQL Plugins

The CodeQL plugins are described in the [plugin readme](https://github.com/microsoft/mu_basecore/blob/release/202208/.pytool/Plugin/CodeQL/Readme.md).
This readme does not repeat information and instead focuses on explaining the context of the plugins within this
platform.

Put simply, the plugins allow a single command-line argument (`--codeql`) to be provided to the normal `stuart`
commands already used in this platform to run CodeQL.

For example:

- `stuart_update --codeql` - Downloads the appropriate CodeQL CLI for your operating system.
- `stuart_build --codeql` - Generates a CodeQL database using that CodeQL during the build. In post-build, the
  database is automatically analyzed and a SARIF file is generated.

Be aware that these commands will take a long time. The CodeQL CLI is several hundred megabytes in size and hooking
CodeQL into the build (1) forces a clean build (2) adds additional CodeQL database logic, both of which increase
the overall time of the build.

  > Note: The CodeQL queries run during analysis by default are those in [MuCodeQlQueries.qls](https://github.com/microsoft/mu_basecore/blob/release/202208/.pytool/Plugin/CodeQL/MuCodeQlQueries.qls).
  >
  > The CodeQL plugin readme describes how to change the queries run and change how the plugin interprets the results.
  > It also describes how to view SARIF results conveniently in an IDE such as Visual Studio Code.
  >
  > Also by default, this platform shows CodeQL result from those queries but does not fail the build if there are
  > any errors.

### CodeQL Database and Result Locations

Although the database and result directory locations are documented in the plugin readme, they are repeated here for
convenience.

---

The CodeQL database is written to a directory unique to the package and target being built:

  `Build/codeql-db-<package>-<target>-<instance>`

For example: `Build/codeql-db-qemuq35pkg-debug-0`

The plugin does not delete or overwrite existing databases, the instance value is simply increased. This is
because databases are large, take a long time to generate, and are important for reproducing analysis results. The user
is responsible for deleting database directories when they are no longer needed.

Similarly, analysis results are written to a directory unique to the package and target. For analysis, results are
stored in individual files so those files are stored in a single directory.

For example, all analysis results for the above package and target will be stored in:
  `codeql-analysis-qemuq35pkg-debug`

CodeQL results are stored in [SARIF](https://sarifweb.azurewebsites.net/) (Static Analysis Results Interchange Format)
([CodeQL SARIF documentation](https://codeql.github.com/docs/codeql-cli/sarif-output/)) files. Each SARIF file
corresponding to a database will be stored in a file with an instance matching the database instance.

For example, the analysis result file for the above database would be stored in this file:
  `codeql-analysis-qemuq35pkg-debug/codeql-db-qemuq35pkg-debug-0.sarif`

The [SARIF Viewer extension for VS Code](https://marketplace.visualstudio.com/items?itemName=MS-SarifVSCode.sarif-viewer)
can open the .sarif file generated by this plugin and allow you to click links directly to the problem area in source
files.
