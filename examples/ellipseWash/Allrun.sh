#!/bin/bash

if ls *.out >/dev/null 2>&1; then
    echo "[ERROR] .out files already exist."
    echo "Please run ./Allclean.sh before continuing."
    exit 1
fi

blockMesh
setFields
gateFoam