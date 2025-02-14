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

pwd
cd `realpath`/build
cmake ..
make -j$(nproc)

# exit the venv
deactivate
