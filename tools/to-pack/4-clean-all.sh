#!/bin/bash

export TEMP_PATH="/home/ultracopier-temp/"
export WINEBASEPATH="/home/wine/"
export ULTRACOPIERSOURCESPATH="/root/ultracopier/sources/"
export BASE_PWD=`pwd` 

export ULTRACOPIER_VERSION=`grep -F "ULTRACOPIER_VERSION" ${ULTRACOPIERSOURCESPATH}/Variable.h | grep -F "0.4" | sed -r "s/^.*([0-9]+\.[0-9]+\.[0-9]+\.[0-9]+).*$/\1/g"`

echo "Clean the temp folder..."
source sub-script/clean-all.sh
cd ${BASE_PWD}
echo "Clean the temp folder... done"

