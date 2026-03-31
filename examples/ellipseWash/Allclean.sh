#!/bin/sh
. ${WM_PROJECT_DIR:?}/bin/tools/RunFunctions

# cleanCase # not exists?

# remove all timesteps
rm -rf [1-9]* 0.[0-9]*

# remove mesh
if [ -d "constant/polyMesh" ]; then
    echo "Cleaning polyMesh, keeping blockMeshDict..."
    find constant/polyMesh -type f ! -name 'blockMeshDict' -delete
fi

# remove intermediate results
rm -rf VTK
rm -rf postProcessing
rm -rf dynamicCode
rm -rf *.obj

# remove logs
rm -f log.*
rm -f *.log
rm -f core
rm -f .*.lock
rm -f cloud.out

echo "Case cleaned successfully."