#!/bin/bash

if ls *.out >/dev/null 2>&1; then
    echo "[ERROR] .out files already exist."
    echo "Please run ./Allclean.sh before continuing."
    exit 1
fi

cp 0/alpha1.org 0/alpha1

blockMesh
setFields
gateFoam