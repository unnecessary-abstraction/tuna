#! /bin/bash

set -e

if [[ "$#" -eq 1 ]]; then
	branch=$1
else
	branch=master-next
fi

git fetch staging --prune
git review staging/$branch
TUNA_MAKE_OPTIONS=-j8 TUNA_BRANCH=staging/$branch ./scripts/test-branch.py
git merge --ff-only staging/$branch
