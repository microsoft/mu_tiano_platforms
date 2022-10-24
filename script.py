from edk2toollib.utility_functions import RunCmd
import argparse
import os
from io import StringIO 


def main():
    parser = argparse.ArgumentParser(description="Creates a PR in each repo that uses this as a subtree.")
    parser.add_argument('--token', '-T', dest="token")

    args = parser.parse_args()
    token = args.token
    
    managed_repos = [
        #{"name": "mu_basecore", "url": "https://github.com/Javagedes/mu_basecore", "branch": "release/202202"},
        {"name": "mu_tiano_platforms", "url": "https://github.com/Javagedes/mu_tiano_platforms", "branch":"release/202202"}
    ]

    branch = 'subtree/github/update'
    path = os.path.join(os.getcwd(), 'src')
    os.makedirs(path, exist_ok=True)

    for repo in managed_repos:
        repo_path = os.path.join(path, repo["name"])
        # clone repo
        params = f'clone {repo["url"]}'
        ret = RunCmd('git', params, workingdir=path, outstream=StringIO())
        print(ret)

        # checkout branch
        params = f'checkout -b {branch}'
        ret = RunCmd('git', params, workingdir=repo_path)
        print(ret)

        # Update the subtree
        params = f'subtree pull --prefix ./.github/ https://github.com/Javagedes/mu_common_github master --squash'
        ret = RunCmd('git', params, workingdir=repo_path)
        print(ret)

        # push the branch
        params = f'-c http.extraheader="AUTHORIZATION: bearer {token}" push origin {branch}'
        ret = RunCmd('git', params, workingdir=repo_path)
        print(ret)

if __name__ == "__main__":
    main()