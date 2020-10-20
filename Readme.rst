============================
Mu Tiano Platform Repository
============================

============================= ================= ===============
 Platform                     VS2017            GCC
============================= ================= ===============
Q35                           |Q35VsBuild|      |Q35GccBuild|
Ovmf                          |OvmfVsBuild|     |OvmfGccBuild|
CoreCI                        |CoreVsBuild|     |CoreGccBuild|
============================= ================= ===============

This repository is a platform repository for Project Mu.
See the RepoDetails.md for more information about this repo.

Please see Project Mu for details https://microsoft.github.io/mu


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

Copyright (c) Microsoft Corporation. All rights reserved.
SPDX-License-Identifier: BSD-2-Clause-Patent

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

.. |Q35VsBuild| image:: https://windowspartners.visualstudio.com/MSCoreUEFI/_apis/build/status/mu_tiano_platforms/CI__VS2019-Q35-Platform?branchName=release%2F202008
.. |Q35GccBuild| image:: https://windowspartners.visualstudio.com/MSCoreUEFI/_apis/build/status/mu_tiano_platforms/CI__GCC5-Q35-Platform?branchName=release%2F202008

.. |OvmfVsBuild| image:: https://windowspartners.visualstudio.com/MSCoreUEFI/_apis/build/status/mu_tiano_platforms/CI__VS2019-OVMF-Platform?branchName=release%2F202008
.. |OvmfGccBuild| image:: https://windowspartners.visualstudio.com/MSCoreUEFI/_apis/build/status/mu_tiano_platforms/CI__GCC5-OVMF-Platform?branchName=release%2F202008

.. |CoreVsBuild| image:: https://windowspartners.visualstudio.com/MSCoreUEFI/_apis/build/status/mu_tiano_platforms/CI__VS2019-Core-Ci?branchName=release%2F202008
.. |CoreGccBuild| image:: https://img.shields.io/badge/coverage-coming_soon-blue
