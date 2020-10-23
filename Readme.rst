============================
Mu Tiano Platform Repository
============================

Core CI Build Status:

=========================== =================== ==================
Host and Toolchain          Build Status        Test Status
=========================== =================== ==================
Windows_VS2019_             |WindowsCiBuild|    |WindowsCiTest|
Ubuntu_GCC5_                |UbuntuCiBuild|     |UbuntuCiTest|
=========================== =================== ==================

Platform Build and Boot Status:

============================= =================
Platform and Toolchain        Status
============================= =================
Q35_VS2019_                   |Q35VsBuild|
Q35_GCC5_                     |Q35GccBuild|
Ovmf_VS2019_                  |OvmfVsBuild|
Ovmf_GCC5_                    |OvmfGccBuild|
============================= =================

This repository is part of Project Mu.  Please see Project Mu for details https://microsoft.github.io/mu

Branch Status - release/202008
==============================

:Status:
  In Development

:Entered Development:
  2020/09/23

:Anticipated Stabilization:
  November 2020

Branch Changes - release/202008
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

2008_RefBoot Changes
--------------------

- N/A

2008_CIBuild Changes
--------------------

- N/A

2008_Rebase Changes
-------------------

| Starting commit: N/A
| Destination commit: N/A

- N/A


Code of Conduct
===============

This project has adopted the Microsoft Open Source Code of Conduct https://opensource.microsoft.com/codeofconduct/

For more information see the Code of Conduct FAQ https://opensource.microsoft.com/codeofconduct/faq/
or contact `opencode@microsoft.com <mailto:opencode@microsoft.com>`_. with any additional questions or comments.

Contributions
=============

Contributions are always welcome and encouraged!
Please open any issues in the Project Mu GitHub tracker and read https://microsoft.github.io/mu/How/contributing/

Feature Branches
================

This repository is used to develop and validate new core features on the Q35 platform.  This repository supports a
feature branch pattern where new features are developed and introduced in a branch.  The feature branch used for
development can serve as an standalone example of feature enablement or it can be merged into the main release branch
after development is complete.

To create a new feature branch:

1. Checkout the latest release branch
2. Create a branch named feature/<your feature name here>
3. Create a `Branch_ReadMe.md` in the docs folder of your new branch.

A template for `Branch_ReadMe.md` can be found in `docs/BranchReadMe_template.md`

Copyright & License
===================

| Copyright (C) Microsoft Corporation
| SPDX-License-Identifier: BSD-2-Clause-Patent

Upstream License (TianoCore)
============================

Copyright (c) 2019, TianoCore and contributors.  All rights reserved.

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

.. _Windows_VS2019: https://dev.azure.com/projectmu/mu/_build/latest?definitionId=58&&branchName=release%2F202008
.. |WindowsCiBuild| image:: https://dev.azure.com/projectmu/mu/_apis/build/status/CI/Mu%20Tiano%20Platforms/Mu%20Tiano%20Platforms%20CI%20VS2019?branchName=release%2F202008
.. |WindowsCiTest| image:: https://img.shields.io/azure-devops/tests/projectmu/mu/58.svg

.. _Ubuntu_GCC5: https://dev.azure.com/projectmu/mu/_build/latest?definitionId=57&branchName=release%2F202008
.. |UbuntuCiBuild| image:: https://dev.azure.com/projectmu/mu/_apis/build/status/CI/Mu%20Tiano%20Platforms/Mu%20Tiano%20Platforms%20CI%20Ubuntu%20GCC5?branchName=release%2F202008
.. |UbuntuCiTest| image:: https://img.shields.io/azure-devops/tests/projectmu/mu/57.svg

.. _Q35_VS2019: https://dev.azure.com/projectmu/mu/_build/latest?definitionId=59&&branchName=release%2F202008
.. |Q35VsBuild| image:: https://dev.azure.com/projectmu/mu/_apis/build/status/CI/Mu%20Tiano%20Platforms/Mu%20Tiano%20Platforms%20Q35%20Plat%20CI%20VS2019?branchName=release%2F202008
.. _Q35_GCC5: https://dev.azure.com/projectmu/mu/_build/latest?definitionId=60&&branchName=release%2F202008
.. |Q35GccBuild| image:: https://dev.azure.com/projectmu/mu/_apis/build/status/CI/Mu%20Tiano%20Platforms/Mu%20Tiano%20Platforms%20Q35%20Plat%20CI%20GCC5?branchName=release%2F202008

.. _Ovmf_VS2019: https://dev.azure.com/projectmu/mu/_build/latest?definitionId=61&&branchName=release%2F202008
.. |OvmfVsBuild| image:: https://dev.azure.com/projectmu/mu/_apis/build/status/CI/Mu%20Tiano%20Platforms/Mu%20Tiano%20Platforms%20OVMF%20Plat%20CI%20VS2019?branchName=release%2F202008
.. _Ovmf_GCC5: https://dev.azure.com/projectmu/mu/_build/latest?definitionId=62&&branchName=release%2F202008
.. |OvmfGccBuild| image:: https://dev.azure.com/projectmu/mu/_apis/build/status/CI/Mu%20Tiano%20Platforms/Mu%20Tiano%20Platforms%20OVMF%20Plat%20CI%20GCC5?branchName=release%2F202008
