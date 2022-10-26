from github import Github
from git import Repo
from edk2toollib.utility_functions import RunCmd
import argparse
import os
from io import StringIO 


def main():
    parser = argparse.ArgumentParser(description="Creates a PR in each repo that uses this as a subtree.")
    parser.add_argument('--token', '-T', dest="token")

    args = parser.parse_args()
    token = args.token
    
    # github = Github(token)

    managed_repos = [
        # {"name": "mu_basecore", "url": "https://github.com/Javagedes/mu_basecore", "branch": "release/202202"},
        {"name": "mu_tiano_platforms", "url": "https://github.com/Javagedes/mu_tiano_platforms.git", "branch":"release/202202"}
    ]

    RunCmd('git', 'config --global user.name "Azure Devops')
    RunCmd('git', 'config --global user.email "joey.vagedes@gmail.com')


    # Note - we re-use the same branch name. This way if the previous PR has not yet been merged, we just add to the existing branch,
    # and thus, the existing PR.
    branch_name = 'subtree/github/update'
    path = os.path.join(os.getcwd(), 'src')
    os.makedirs(path, exist_ok=True)

    for repo in managed_repos:
        
        # Clone the repository
        r = Repo.clone_from(repo["url"], os.path.join(path, repo["name"]))
        
        # Checkout the branch if it exists, or create one if it does not.
        branch = [b for b in r.branches if b.name == branch_name]
        if not branch:
            branch = r.create_head(branch_name)
        branch.checkout()

        r.git.subtree('pull', '--prefix', '.github/', 'https://github.com/Javagedes/mu_common_github', 'master', '--squash')


        
        # Update the subtree

        # repo_path = os.path.join(path, repo["name"])
        # clone repo
        # params = f'clone {repo["url"]}'
        # ret = RunCmd('git', params, workingdir=path, outstream=StringIO())
        # print(ret)

        # checkout branch
        # params = f'checkout -b {branch}'
        # ret = RunCmd('git', params, workingdir=repo_path)
        # print(ret)

        # Update the subtree
        # params = f'subtree pull --prefix ./.github/ https://github.com/Javagedes/mu_common_github master --squash'
        # ret = RunCmd('git', params, workingdir=repo_path)
        # print(ret)

        # push the branch
        # TODO: Does not like the current way of pushing
        # params = f'push https://{token}@github.com/Javagedes/mu_tiano_platforms.git {branch}'
        # ret = RunCmd('git', params, workingdir=repo_path)
        # print(ret)

if __name__ == "__main__":
    main()