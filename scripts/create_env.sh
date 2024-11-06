#!/usr/bin/env bash

# -e = exit on failure -- makes it more obvious when this script is not going to work
set -e

# realpath function to allow mac compatibility
realpath() {
    [[ $1 = /* ]] && echo "$1" || echo "$PWD/${1#./}"
}

# Determine where the virtual environment should be located
WORKDIR=`realpath "pyframework"`

# Check if things are installed by trying an import
# If one of these fail, the overall script will fail
echo "python3 dev headers, pip, venv are required"
python3 --version
python3 -c 'import pip'
python3 -c 'import venv'

# For ubuntu, run `sudo apt install python3 python3-dev python3-pip python3-venv`
# Other options include using `brew` on Mac OS or miniconda on Linux or Mac

# Create environment in workdir
if ! python3 -m venv "$WORKDIR"; then
    echo "Could not create venv in $WORKDIR"
    exit
fi

# Activate the environment
echo "$WORKDIR/bin/activate"
source "$WORKDIR/bin/activate"

# Show we are using correct venv python
which python

# Get the python verison
PYVER=`python -c 'import sys; print("python{}.{}".format(sys.version_info[0], sys.version_info[1]))'`

echo "Python Version: $PYVER"

# Make sure pip is updated
pip install --upgrade pip
pip install -r requirements.txt

declare -a Repos=("caspian" "eons")

if [ -d .git ]; then
    # Get the correct root URL from the Framework's url.
    FRAMEWORK_URL=`git remote get-url origin`
    if [[ "$FRAMEWORK_URL" == *"code.ornl.gov"* ]]; then
        REPO_ROOT_URL=git@code.ornl.gov:neuromorphic-computing
    else
        REPO_ROOT_URL=git@bitbucket.org:neuromorphic-utk
    fi
    [ -d "eons/include" ] || git clone ${REPO_ROOT_URL}/eons.git
    if [[ "$FRAMEWORK_URL" == *"code.ornl.gov"* ]]; then
        [ -d "./processors/caspian" ] || git clone git@code.ornl.gov:neuromorphic-computing/caspian.git ./processors/caspian
    fi
#[ -d "./processors/gnp" ] || git clone git@code.ornl.gov:neuromorphic-computing/gnp.git ./processors/gnp
fi

# git clone git@bitbucket.org:neuromorphic-utk/fw6gnp.git gnp

. scripts/update_env.sh
