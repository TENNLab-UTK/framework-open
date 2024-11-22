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

. scripts/update_env.sh
