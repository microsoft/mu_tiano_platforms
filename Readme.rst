============================
Mu Tiano Platform Repository
============================

Core CI Build Status:

=========================== =================== ==================
Host and Toolchain          Build Status        Test Status
=========================== =================== ==================
Windows_VS2022_             |WindowsCiBuild|    |WindowsCiTest|
Ubuntu_GCC5_                |UbuntuCiBuild|     |UbuntuCiTest|
=========================== =================== ==================

Platform Build and Boot Status:

============================= =================
Platform and Toolchain        Status
============================= =================
Q35_VS2022_                   |Q35VsBuild|
Q35_GCC5_                     |Q35GccBuild|
SBSA_GCC5_                    |SBSAGccBuild|
============================= =================

This repository is part of Project Mu.  Please see Project Mu for details https://microsoft.github.io/mu

Branch Status - release/202202
==============================

:Status:
  In Development

:Entered Development:
  2022/03/10

:Anticipated Stabilization:
  May 2022

Branch Changes - release/202202
===============================

Breaking Changes-dev
--------------------

- None

Main Changes-dev
----------------

- None

Bug Fixes-dev
-------------

- None

2202_RefBoot Changes
--------------------

- None

2202_CIBuild Changes
--------------------

- None

2202_Rebase Changes
-------------------

| Starting commit: 0e84c00d
| Destination commit: 64d9590a

- None

Getting Started
===============

The best way to get started is to review the details of this repository and look at the platforms in this repo.

- `Repo Details <RepoDetails.md>`_

Platforms can be found under the `Platforms` folder inside the root of this repo.

Details about the platform and how to build/use each platform can be found in their respective directories.

- `Q35 Platform <Platforms/QemuQ35Pkg/Docs/QemuQ35_ReadMe.md>`_
- `Ovmf upstream Platform <Platforms/OvmfPkg/ReadMe.md>`_

Feature Branches
================

This repository is used to develop and validate new core features on the Q35 platform.  This repository supports a
feature branch pattern where new features are developed and introduced in a branch.  The feature branch used for
development can serve as an standalone example of feature enablement or it can be merged into the main release branch
after development is complete.

To create a new feature branch:

1. Checkout the latest release branch
2. Create a branch named feature/{release you branched from}/<your feature name here>
3. Create a `Branch_ReadMe.md` in the docs folder of your new branch.

A template for `Branch_ReadMe.md` can be found in `docs/BranchReadMe_template.md`

An example of a feature branch name would be feature/202202/sbat.
The feature is sbat and it is based on release/202202.

Features must be "ON by default" and QemuQ35Pkg tries to avoid switches where possible. If your feature is required
to boot to an OS, please make changes to OvmfPkg which is considered the minimum platform.

Feature branches over two releases old will be archived. For example, feature/202002/sbat would get archived
when release/202102 was released, by moving it to archived/feature/202002/sbat. As such, feature branch owners
will need to rebase their feature on top of release branches as they are released and publish a new feature branch.

Example Branches
================

Example branches are different from feature branches in that they are not meant to merge into Q35 eventually.
The goal is for them to be shared as an example of how to integrate a specific feature.
Naming follows feature branches so an example example branch would be example/202102/prm.
In the example branch, please create a Branch_ReadMe.md in the docs folder, similar to a feature branch.
However, in your Branch_ReadMe.md, please specify that this is an example branch.

Commits should be laid out in an easy to understand manner with an empty commit that specifies that the example starts after the empty commit.
This can be done by running git commit --allow-empty -m "Example of {feature} starts here".
This will allow those you share it with to easily understand what commits are relevant.

Example branches also follow the same archiving process, so example branches will only last two releases.
Example branch owners will need to rebase their example branches for them to stay current.

Repo Maintenance
================

Upstream Sync Details
---------------------

The `upstream/main` branch should be automatically maitained by the script in the `upstream/sync` branch. These branches are
primarily on the internal repo and maintained by an internal pipeline.

In each commit, the "MU SOURCE COMMIT" comment at the end is the equivalent of the "cherry-picked from..." comment in a
`git cherry-pick -x ...` command.

For the integration process, the target commit should be selected and tagged `XXXX_Upstream` and pushed to all mirrors.

Code of Conduct
===============

This project has adopted the Microsoft Open Source Code of Conduct https://opensource.microsoft.com/codeofconduct/

For more information see the Code of Conduct FAQ https://opensource.microsoft.com/codeofconduct/faq/
or contact `opencode@microsoft.com <mailto:opencode@microsoft.com>`_. with any additional questions or comments.

Contributions
=============

Contributions are always welcome and encouraged!
Please open any issues in the Project Mu GitHub tracker and read https://microsoft.github.io/mu/How/contributing/

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

.. _Windows_VS2022: https://dev.azure.com/projectmu/mu/_build/latest?definitionId=58&&branchName=main
.. |WindowsCiBuild| image:: https://dev.azure.com/projectmu/mu/_apis/build/status/CI/Mu%20Tiano%20Platforms/Mu%20Tiano%20Platforms%20CI%20VS?branchName=main
.. |WindowsCiTest|  image:: https://img.shields.io/azure-devops/tests/projectmu/mu/58.svg

.. _Ubuntu_GCC5: https://dev.azure.com/projectmu/mu/_build/latest?definitionId=57&branchName=main
.. |UbuntuCiBuild| image:: https://dev.azure.com/projectmu/mu/_apis/build/status/CI/Mu%20Tiano%20Platforms/Mu%20Tiano%20Platforms%20CI%20Ubuntu%20GCC5?branchName=main
.. |UbuntuCiTest|  image:: https://img.shields.io/azure-devops/tests/projectmu/mu/57.svg

.. _Q35_VS2022: https://dev.azure.com/projectmu/mu/_build/latest?definitionId=59&&branchName=main
.. |Q35VsBuild| image:: https://dev.azure.com/projectmu/mu/_apis/build/status/CI/Mu%20Tiano%20Platforms/Mu%20Tiano%20Platforms%20Q35%20Plat%20CI%20VS?branchName=main
.. _Q35_GCC5:   https://dev.azure.com/projectmu/mu/_build/latest?definitionId=60&&branchName=main
.. |Q35GccBuild| image:: https://dev.azure.com/projectmu/mu/_apis/build/status/CI/Mu%20Tiano%20Platforms/Mu%20Tiano%20Platforms%20Q35%20Plat%20CI%20GCC5?branchName=main

.. _SBSA_GCC5:   https://dev.azure.com/projectmu/mu/_build/latest?definitionId=138&&branchName=main
.. |SBSAGccBuild| image:: https://dev.azure.com/projectmu/mu/_apis/build/status/CI/Mu%20Tiano%20Platforms/Mu%20Tiano%20Platforms%20SBSA%20Plat%20CI%20GCC5?branchName=main
