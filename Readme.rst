============================
Mu Tiano Platform Repository
============================

|Latest Mu Tiano Platform Release Version (latest SemVer)| |Commits Since Last Release|

=========================== =================== ==================
Host and Toolchain          Build Status        Test Status
=========================== =================== ==================
`Windows Visual Studio`_    |WindowsCiBuild|    |WindowsCiTest|
`Ubuntu GCC5`_              |UbuntuCiBuild|     |UbuntuCiTest|
=========================== =================== ==================

============================= =================
Platform and Toolchain        Status
============================= =================
`Q35 Visual Studio`_          |Q35VsBuild|
`Q35 GCC5`_                   |Q35GccBuild|
`SBSA GCC5`_                  |SBSAGccBuild|
============================= =================

.. |Latest Mu Tiano Platform Release Version (latest SemVer)| image:: https://img.shields.io/github/v/release/microsoft/mu_tiano_platforms?label=Latest%20Release
   :target: https://github.com/microsoft/mu_tiano_platforms/releases/latest

.. |Commits Since Last Release| image:: https://img.shields.io/github/commits-since/microsoft/mu_tiano_platforms/latest/main?include_prereleases
   :target: https://github.com/microsoft/mu_tiano_platforms/releases

----

Quick Links
===========

- `Build Instructions <Platforms/QemuQ35Pkg/Docs/Development/building.md>`_
- `Contribution Instructions <CONTRIBUTING.md>`_
- `Questions and Other Discussion <https://github.com/microsoft/mu_tiano_platforms/discussions>`_
- `Releases <https://github.com/microsoft/mu_tiano_platforms/releases>`_
- `Submit and View Bugs, Doc Requests, Feature Requests <https://github.com/microsoft/mu_tiano_platforms/issues>`_
- `Security Issue Reporting Procedure <https://github.com/microsoft/mu_tiano_platforms/security/policy>`_

About This Repo
===============

Mu Tiano Platform is a public repository of `Project Mu`_ based firmware that targets the open-source `QEMU`_
processor emulator.

This repository provides readily available, free, and feature rich platforms that serve as an example for feature
enablement and validation, demonstrating how a single firmware codebase can be shared across multiple products and
architectures, promoting serviceable, maintainable, up-to-date and secure firmware.

A goal of this repository is to reduce the overhead of testing and evaluating common functionality before deployment
to physical hardware.

Current Platforms Supported
---------------------------

- `QemuQ35Pkg`_

  - `QemuQ35Pkg Detailed Info`_

  - Intel Q35 chipset with ICH9 south bridge

- `QemuSbsaPkg`_

  - `QemuSbsaPkg Detailed Info`_

  - ARM Server Base System Architecture

.. _`Project Mu`: https://microsoft.github.io/mu
.. _`QEMU`: https://www.qemu.org/
.. _`QemuQ35Pkg`: Platforms/QemuQ35Pkg
.. _`QemuQ35Pkg Detailed Info`: Platforms/QemuQ35Pkg/Docs/QemuQ35_ReadMe.md
.. _`QemuSbsaPkg`: Platforms/QemuSbsaPkg
.. _`QemuSbsaPkg Detailed Info`: Platforms/QemuSbsaPkg/Docs/QemuSbsa_ReadMe.md

Getting Started
===============

Individual platforms are maintained in the `Platforms` directory. Each platform directory has a readme file and a
`Docs` folders with more detailed platform-specific information.

Build Instructions
------------------

1. If you are new to the "stuart" build system in general refer to the following for a general overview
   `How to Build in edk2 with Stuart`_.

2. For instructions specific to this repo refer to `building QemuQ35Pkg`_.

You can then apply that knowledge to build the platform you're interested in with the `PlatformBuild.py` files located
in the platform directory.

.. _`Building QemuQ35Pkg`: Platforms/QemuQ35Pkg/Docs/Development/building.md
.. _`How to Build in edk2 with Stuart`: https://github.com/tianocore/tianocore.github.io/wiki/How-to-Build-With-Stuart

Releases
--------

It is recommended to consume binaries built from the code in this repo (outside of development purposes) through a
release.

Releases are tagged in the repository and are available for download from the `Releases`_ page. Each release contains
release notes describing the changes since the last release that highlight important changes such as breaking changes.

A semantic versioning process (version is `<major.minor.patch>`) is followed with the following rules:

- Major Version

  - A major version change indicates a breaking change. This means that the release is not backward
    compatible with the previous release. This is typically a change to the API or ABI of a component.

- Minor Version

  - A minor version change indicates a new feature or enhancement. This means that the release is backward
    compatible with the previous release but includes new functionality or a major rework of existing functionality.

- Patch Version

  - A patch version change indicates a bug fix or any other change. This means that the release is backward compatible
    with the previous release and contains no new functionality.

Every release includes `DEBUG` and `RELEASE` binaries for all supported platforms. `DEBUG` is recommended to debug
a release and `RELEASE` used for non-debug scenarios. If you file a bug and a "debug log" is requested, that needs to
be produced from the `DEBUG` build of the release.

Every release is published as both a `NuGet`_ package and a zip file. Both formats contain the exact same content and
both can be manually downloaded or used in an automated build environment.

NuGet packages are published in the `Mu Tiano Platforms GitHub NuGet Feed`_ and zip files are attached as "assets" to
the release on GitHub.

- Zip File Usage Example: Scroll to the bottom of the `v1.0.0`_ release page and you will see an "Assets"
  section that contains the zip files for that release. You can manually download the appropriate zip file there.

- NuGet Package Usage Example: A tool like `dependabot`_ can easily hook into the NuGet feed to automatically download
  releases into a project based on the semantic versioning rules described above.

Outside of topics that should be filed as an issue, please feel free to ask questions about the project in general or
a particular release in the `Discussions`_ area.

.. _`v1.0.0`: https://github.com/microsoft/mu_tiano_platforms/releases/tag/v1.0.0
.. _`dependabot`: https://docs.github.com/en/code-security/dependabot/dependabot-version-updates/configuring-dependabot-version-updates
.. _`Discussions`: https://github.com/microsoft/mu_tiano_platforms/discussions/categories/general
.. _`Mu Tiano Platforms GitHub NuGet Feed`: https://github.com/orgs/microsoft/packages?repo_name=mu_tiano_platforms
.. _`NuGet`: https://learn.microsoft.com/nuget/what-is-nuget

Feature Branches
----------------

1. Create a branch for involved feature development on your fork of the repository.

2. When the feature is ready, submit a pull request.

You can choose to squash all commits into one feature commit into `mu_tiano_platforms/main` or prepare
a series of commits exactly as you would like them to appear and the pull request will be rebased on top of the
`mu_tiano_platforms/main` branch.

A template for `Branch_ReadMe.md` can be found in `docs/BranchReadMe_template.md`

Features must be "on by default" in all applicable platforms to avoid build switch complexity that leads to stale
feature coverage.

Repository Origin
=================

`QemuQ35Pkg` in this repository was originally derived from `OvmfPkg` in TianoCore. The package is considered
stable so regular syncing is not performed with the upstream package. Select changes are cherry-picked based on
functional or security importance. Additional cherry picks are welcome if they are necessary for you to be productive
with the platform  in this repository.

⚠️ Security Warning
===================

This repository and all code within it is not part of an officially supported customer facing product and therefore
long term servicing is not supported. All code in this repository is provided as-is and is not intended to be used
in a production system and may not be suitable in a production system.

Code of Conduct
===============

This project has adopted the Microsoft Open Source Code of Conduct https://opensource.microsoft.com/codeofconduct/

For more information see the Code of Conduct FAQ https://opensource.microsoft.com/codeofconduct/faq/
or contact `opencode@microsoft.com <mailto:opencode@microsoft.com>`_. with any additional questions or comments.

Copyright & License
===================

| Copyright (C) Microsoft Corporation
| SPDX-License-Identifier: BSD-2-Clause-Patent

Upstream License (TianoCore)
============================

Copyright (c) 2022, TianoCore and contributors.  All rights reserved.

SPDX-License-Identifier: BSD-2-Clause-Patent

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

Subject to the terms and conditions of this license, each copyright holder
and contributor hereby grants to those receiving rights under this license
a perpetual, worldwide, non-exclusive, no-charge, royalty-free, irrevocable
(except for failure to satisfy the conditions of this license) patent
license to make, have made, use, offer to sell, sell, import, and otherwise
transfer this software, where such license applies only to those patent
claims, already acquired or hereafter acquired, licensable by such copyright
holder or contributor that are necessarily infringed by:

(a) their Contribution(s) (the licensed copyrights of copyright holders and
    non-copyrightable additions of contributors, in source or binary form)
    alone; or

(b) combination of their Contribution(s) with the work of authorship to
    which such Contribution(s) was added by such copyright holder or
    contributor, if, at the time the Contribution is added, such addition
    causes such combination to be necessarily infringed. The patent license
    shall not apply to any other combinations which include the
    Contribution.

Except as expressly stated above, no rights or licenses from any copyright
holder or contributor is granted under this license, whether expressly, by
implication, estoppel or otherwise.

DISCLAIMER

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

.. ===================================================================
.. This is a bunch of directives to make the README file more readable
.. ===================================================================

.. _`Windows Visual Studio`: https://dev.azure.com/projectmu/mu/_build/latest?definitionId=58&&branchName=main
.. |WindowsCiBuild| image:: https://dev.azure.com/projectmu/mu/_apis/build/status/CI/Mu%20Tiano%20Platforms/Mu%20Tiano%20Platforms%20CI%20VS?branchName=main
.. |WindowsCiTest|  image:: https://img.shields.io/azure-devops/tests/projectmu/mu/58.svg

.. _`Ubuntu GCC5`: https://dev.azure.com/projectmu/mu/_build/latest?definitionId=57&branchName=main
.. |UbuntuCiBuild| image:: https://dev.azure.com/projectmu/mu/_apis/build/status/CI/Mu%20Tiano%20Platforms/Mu%20Tiano%20Platforms%20CI%20Ubuntu%20GCC5?branchName=main
.. |UbuntuCiTest|  image:: https://img.shields.io/azure-devops/tests/projectmu/mu/57.svg

.. _`Q35 Visual Studio`: https://dev.azure.com/projectmu/mu/_build/latest?definitionId=59&&branchName=main
.. |Q35VsBuild| image:: https://dev.azure.com/projectmu/mu/_apis/build/status/CI/Mu%20Tiano%20Platforms/Mu%20Tiano%20Platforms%20Q35%20Plat%20CI%20VS?branchName=main
.. _`Q35 GCC5`:   https://dev.azure.com/projectmu/mu/_build/latest?definitionId=60&&branchName=main
.. |Q35GccBuild| image:: https://dev.azure.com/projectmu/mu/_apis/build/status/CI/Mu%20Tiano%20Platforms/Mu%20Tiano%20Platforms%20Q35%20Plat%20CI%20GCC5?branchName=main

.. _`SBSA GCC5`:   https://dev.azure.com/projectmu/mu/_build/latest?definitionId=138&&branchName=main
.. |SBSAGccBuild| image:: https://dev.azure.com/projectmu/mu/_apis/build/status/CI/Mu%20Tiano%20Platforms/Mu%20Tiano%20Platforms%20SBSA%20Plat%20CI%20GCC5?branchName=main
