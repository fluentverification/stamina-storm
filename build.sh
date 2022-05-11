#!/bin/bash

############################################
#
# Bash script to compile stamina with storm on UNIX-like OSs
#
# Created by Josh Jeppson on 8/20/2021
#
############################################

set -e

echo "[MESSAGE] Checking if STORM exists in the STAMINA root directory"

STAMINA_ROOT=pwd
NPROC="$(nproc --all)"

if [! -d storm ]
then
	echo "[MESSAGE] STORM does not appear to be downloaded. Getting STORM"
	git clone https://github.com/moves-rwth/storm
	git checkout stable
	cd storm
	echo "[MESSAGE] Building STORM"
	mkdir build
	cd build
	cmake ..
	make -j$NPROC
	echo "[MESSAGE] Finished building STORM"
	cd ..
	STORM_PATH=pwd
else
	echo "[MESSAGE] STORM already appears to exist. Will not download"
fi

cd $STAMINA_ROOT

echo "[MESSAGE] Building STAMINA"

mkdir build
cd build
cmake .. -DSTORM_PATH=$STORM_PATH
make -j$NPROC

echo "[MESSAGE] Finished building STAMINA"
