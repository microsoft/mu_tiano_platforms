# @file
# A common helper script for any platform in the repository, that is used to generate
# the necessary PCDs for secureboot.
#
# Copyright (c) Microsoft Corporation.
# SPDX-License-Identifier: BSD-2-Clause-Patent
##
from edk2toolext.environment.plugintypes.uefi_helper_plugin import IUefiHelperPlugin
import logging
from pathlib import Path
from edk2toollib.utility_functions import RunPythonScript
import tempfile

class BuildSecurebootPcds(IUefiHelperPlugin):
    def RegisterHelpers(self, obj):
        fp = str(Path(__file__).absolute())
        obj.Register("generate_secureboot_pcds", BuildSecurebootPcds.generate_pcds, fp)
        return 0
    
    @staticmethod
    def generate_pcds(thebuilder) -> int:
        """Generates Secureboot PCDs at the requested location."""
        secureboot_bin_dir = thebuilder.env.GetValue("SECUREBOOT_BINARIES", "")
        if secureboot_bin_dir == "":
            logging.error("SECUREBOOT_BINARIES_PATH is not set")
            return -1
        secureboot_bin_dir = Path(secureboot_bin_dir)

        secureboot_pcd_dir = Path(thebuilder.env.GetValue("WORKSPACE"), "QemuPkg", "AutoGen")
        secureboot_pcd_dir.mkdir(parents=True, exist_ok=True)

        pcd_map = [
            {
                'pcd': 'gMsCorePkgTokenSpaceGuid.PcdDefaultPk',
                'inc_name': 'DefaultPk.inc',
                'cert_path': [Path(secureboot_bin_dir, 'DefaultPk.bin')]
            },
            {
                'pcd': 'gMsCorePkgTokenSpaceGuid.PcdDefaultDb',
                'inc_name': 'DefaultDb.inc',
                'cert_path': [Path(secureboot_bin_dir, 'DefaultDb.bin')]
            },
            {
                'pcd': 'gMsCorePkgTokenSpaceGuid.PcdDefault3PDb',
                'inc_name': 'Default3PDb.inc',
                'cert_path': [Path(secureboot_bin_dir, 'Default3PDb.bin')]
            },
            {
                'pcd': 'gMsCorePkgTokenSpaceGuid.PcdDefaultDbx',
                'inc_name': 'DefaultDbx.inc',
                'cert_path': [Path(secureboot_bin_dir, 'DefaultDbx.bin')]
            },
            {
                'pcd': 'gMsCorePkgTokenSpaceGuid.PcdDefaultKek',
                'inc_name': 'DefaultKek.inc',
                'cert_path': [Path(secureboot_bin_dir, 'DefaultKek.bin')]
            },
        ]

        tmp_dir = Path(tempfile.mkdtemp())
        for entry in pcd_map:
            params = ""
            for cert in entry['cert_path']:
                params +=f' -i "{str(cert)}"'
            params += f' -p {entry["pcd"]}'
            params += f' -o {str(tmp_dir / entry["inc_name"])}'

            ret = RunPythonScript("BinToPcd.py", params)
            if (ret != 0):
                logging.critical("Failed to generate " + entry['pcd'] + " PCD include.")
                return ret
        
        out_file = secureboot_pcd_dir / "SecurebootPcds.inc"
        with open(out_file, 'w') as f:
            for file in tmp_dir.glob("*.inc"):
                with open(file, 'r') as inc:
                    f.write(inc.read())
                    f.write("\n")

        return 0
