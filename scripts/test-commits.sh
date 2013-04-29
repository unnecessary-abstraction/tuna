#! /bin/bash
#
# Test that all commits in a given range are good

set -e
RANGE=$1

# Lets not break the current working directory
SOURCEDIR=`pwd`
WORKINGDIR=`mktemp -d`
trap 'rm -rf $WORKINGDIR' EXIT
cd "$WORKINGDIR"

rsync -a "$SOURCEDIR/" .

for COMMIT in $(git log --reverse --format=format:%H $RANGE); do
    git checkout ${COMMIT} ;
    make clean 1>/dev/null || { echo "$COMMIT: Failed make clean";  break; }
    make all 1>/dev/null || { echo "$COMMIT: Failed make all";  break; }
    make test 1>/dev/null || { echo "$COMMIT: Failed make test";  break; }
done

cd "$SOURCEDIR"
rm -rf "$WORKINGDIR"
