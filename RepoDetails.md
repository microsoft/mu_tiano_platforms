# Mu Tiano Platforms Repository

??? info "Git Details"
    Repository Url: {{mu_tiano_platforms.url}}  
      Branch:         {{mu_tiano_platforms.branch}}  
      Commit:         [{{mu_tiano_platforms.commit}}]({{mu_tiano_platforms.commitlink}})  
      Commit Date:    {{mu_tiano_platforms.date}}  

This is a Project Mu **"Platform"** repository. It contains the edk2 OvmfPkg which supports IA32/X64 virtual firmware
for QEMU.  It also contains a Project Mu QEMU Q35 variant that is customized to enable many of the features of Project Mu.

The idea behind this repository is to provide a free, open source, sample reference platform for enabling Project Mu features.
Eventually there may be numerous feature branches that demonstrate how to enable advanced capabilities.  By providing an
end-to-end example, these features can be easily tested and evaluated before being integrated into other platforms.

## Upstream Alignment

This repo has a filtered version of edk2 as an upstream, which can be found in the `upstream/main` branch. The Azure
Pipeline definition, which performs the upstream syncing process, can be found in `upstream/sync`. When edk2 is tagged,
a new branch with the corresponding tag will be created (example: `upstream/202005`). Since it is a filtered branch, the
exact commit that edk2 is tagged at won't be represented but it will be the closest applicable commit before edk2 was tagged.

In addition, the current release branch of this repo will be rebased on those upstream release commits and a new release
branch will be created and set as default. For example `upstream/202005` will have a development branch of `release/202005`.
The expectation is that only the latest release branch is in active development.

!!! Warning "Long Term Support/Security Patches"
    This repository and all code within it is not part of an officially supported customer
    facing product and therefore long term servicing will not be supported.  Security issues
    will also not be backported.

## More Info

Buildable platforms can be found under the `Platforms` folder inside the root of this repo.
Details about the platform and how to build/use each platform can be found in their respective directories.

- [Q35 Platform](Platforms/QemuQ35Pkg/ReadMe.md)
- [Ovmf upstream Platform](Platforms/OvmfPkg/ReadMe.md)

For general details about Project Mu please see the Project Mu docs (<https://github.com/Microsoft/mu>).

## Microsoft Open Source Code of Conduct

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).

Resources:

- [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/)
- [Microsoft Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/)
- Contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with questions or concerns

## Issues

Please open any issues in the Project Mu GitHub tracker.  
[More Details](https://microsoft.github.io/mu/How/contributing/)

## Contributing Code or Docs

Open a PR.

Please follow the general Project Mu Pull Request process.  
[More Details](https://microsoft.github.io/mu/How/contributing/)

- [Code Requirements](/CodeDevelopment/requirements)
- [Doc Requirements](/DeveloperDocs/requirements)

## Copyright & License

Project Mu contributions will be licensed as BSD-2-Clause-Patent and will contain the SPDX-License-Identifier.

```text
Copyright (C) Microsoft Corporation
SPDX-License-Identifier: BSD-2-Clause-Patent
```

Most of this code comes from the upstream Tianocore Edk2 project and is licensed as described below.

### Upstream License (TianoCore)

Copyright (c) Microsoft Corporation. All rights reserved.
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

================================================================================

Some files are subject to the following license, the MIT license. Those files
are located in:

- Platform/OvmfPkg/Include/IndustryStandard/Xen/
- Platform/OvmfPkg/XenBusDxe/

SPDX-License-Identifier: MIT

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice (including the next
paragraph) shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
