from github import Github
from git import Repo
import argparse
import os
import logging

def parse_args():
    parser = argparse.ArgumentParser(description="Creates a PR in each repo that uses this as a subtree.")
    parser.add_argument('--token', '-T', dest="token")
    parser.add_argument('--user', '-U', dest="user")

    args = parser.parse_args()
    return args

def main():
    args = parse_args()
    token = args.token
    user = args.user

    managed_repos = [
        # {"name": "mu_basecore", "url": "https://github.com/Javagedes/mu_basecore", "base": "release/202202"},
        {"name": "mu_tiano_platforms", "url": "https://github.com/Javagedes/mu_tiano_platforms.git", "base":"release/202202"}
    ]

    g = Github(token)
    pr_body = '''
## Description

Update .github subtree

## How this was tested

Automatically generated PR
'''

    # Note - we re-use the same branch name. This way if the previous PR has not yet been merged, we just add to the existing branch,
    # and thus, the existing PR.
    head = 'subtree/github/update'
    path = os.path.join(os.getcwd(), 'src')
    os.makedirs(path, exist_ok=True)

    # Uses git (GitPython) for managing git commands
    # Uses github (PyGithub) for interfacing with github
    for repo in managed_repos:
        print(f'Starting update for {repo["name"]}')
        create_pr = False
        
        # Clone the repository
        logging.info(f'cloning {repo["name"]}')
        r = Repo.clone_from(repo["url"], os.path.join(path, repo["name"]))
        
        # Checkout the branch if it exists, or create one if it does not.
        # If branch exists, assume PR has been created already and we just 
        # need to update the commit.
        print('Checking out subtree branch')
        branch = [b for b in r.branches if b.name == head]
        if len(branch) == 0:
            print('Branch did not exist; creating new branch')
            create_pr = True
            branch = r.create_head(head)
        else:
            branch = branch[0]
        branch.checkout()

        print('Updating the subtree')
        # Update the subtree, adds the commits to branch so no need to run commit command
        r.git.subtree('pull', '--prefix', '.github/', 'https://github.com/Javagedes/mu_common_github', 'master', '--squash')

        # Push the commit
        print("Pushing the commit")
        r.git.push(f'https://{user}:{token}@{repo["url"].lstrip("https://")}', branch.name, "--force")

        # Create PR
        if create_pr:
            print("Branch did not exist; creating new PR")
            github_repo = g.get_repo(f'{repo["url"].lstrip("https:://github.com/").rstrip(".git")}')
            github_repo.create_pull(title="Update .github subtree", body = pr_body, head = head, base = repo["base"])

if __name__ == "__main__":
    main()