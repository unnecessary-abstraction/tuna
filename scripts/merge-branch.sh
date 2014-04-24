#! /bin/bash

set -e

branch=$1

git fetch staging --prune
git review staging/$branch
TUNA_MAKE_OPTIONS=-j8 TUNA_BRANCH=staging/$branch ./scripts/test-branch.py
git merge --ff-only staging/$branch
