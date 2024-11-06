#!/usr/bin/env bash

# -e = exit on failure
set -e

# realpath function to allow mac compatibility
realpath() {
    [[ $1 = /* ]] && echo "$1" || echo "$PWD/${1#./}"
}

WORKDIR=`realpath pyframework`

# Activate the environment
source $WORKDIR/bin/activate

# Show we are using correct venv python
PYLOC=`which python`
PYVER=`python3 -c 'import sys; print("python{}.{}".format(sys.version_info[0], sys.version_info[1]))'`

# A quick fix for Mac
if [ `uname` == "Linux" ]; then
    #NUM_CORES=`cat /proc/cpuinfo | awk '/^processor/{print $3}' | wc -l | awk '{print $0/2}'`
    NUM_CORES=`cat /proc/cpuinfo | awk '/^cpu cores/{print $4}' | head -n 1`
    echo "Detected Linux"
else
    # TODO: Fix for Mac?
    # NUM_CORES=`system_profiler | awk '/Number Of CPUs/{print $4}{next;}'`
    NUM_CORES=4
    echo "Detected non-Linux -- default to 4 threads"
fi

echo "Compile with $NUM_CORES threads"
echo "Using $PYVER at $PYLOC"

if [ -d .git ]; then
    FRAMEWORK_URL=`git remote get-url origin`
    if [[ "$FRAMEWORK_URL" == *"code.ornl.gov"* ]] || [[ "$FRAMEWORK_URL" == *"bitbucket"*"framework"* ]]; then
        declare -a Projects=("." "eons" "processors/ravens" "processors/risp" "cpp-apps" "cpp-apps/applications/bowman"  "cpp-apps/applications/asteroids" "cpp-apps/applications/polebalance")
    fi
else
    declare -a Projects=("." "eons" "processors/ravens" "processors/risp" "cpp-apps/applications/polebalance")
fi

# Check for Caspian
if [ -d "processors/caspian" ]; then
    Projects+=("processors/caspian")
else
    echo "Caspian repository not found."
fi

# Optionally do a clean build
if [ "$1" == "clean" ]; then
    for prj in ${Projects[@]}; do
        make -C $prj clean
    done
fi

# Build each project assuming it follows a standard format.
#   Project repositories should:
#     - Be cloned in the top level framework directory
#     - Use standard makefiles with a `python` target
#     - Create a .so file in a `build` directory with its Python extension
for prj in ${Projects[@]}; do
    make -C $prj python -j$NUM_CORES
    cp $prj/build/*.so $WORKDIR/lib/$PYVER/site-packages/.
done

# If the building_blocks repo is pulled, then install it.
if [ -d "./building_blocks/packages" ]; then
    python3 -m pip install building_blocks/packages/pyblocks
    python3 -m pip install building_blocks/packages/composition
else
    echo "building_blocks repository not found."
fi

# If the Whetstone repo is pulled then we need to install some additional
# python packages.
if [ -d "./Whetstone" ]; then
    python3 -m pip install keras
    python3 -m pip install tensorflow
    python3 -m pip install ./Whetstone
else
    echo "Whetstone repository not found."
fi

# exit the venv
deactivate
