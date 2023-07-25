#!/bin/sh

############################################
#
# Shell script to compile stamina with storm on UNIX-like OSs
#
# Created by Josh Jeppson on 8/20/2021. Rewritten on 5/26/2023
#
############################################

set -e

export STORM_URL="https://github.com/moves-rwth/storm"
export STAMINA_URL="https://github.com/fluentverification/stamina-storm"
# Ignore one processor so that the user's computer doesn't freeze
export PROCS=$(nproc --ignore=1)
export ROOT_DIR=$(pwd)

function msg() {
	echo "[MESSAGE] $1"
}

function mainFnStamina() {
	cd $ROOT_DIR
	if [ -z "$STORM_PATH" ];
	then
		msg "Storm does not appear to be installed. If this is not the case, please set the STORM_PATH environment variable"
		msg "Installing Storm in $ROOT_DIR/storm"
		git clone $STORM_URL storm
		buildStorm
		msg "Finished installing Storm"
	else
		msg "STORM appears to be installed at '$STORM_PATH'"
	fi
	buildStamina
}

function buildStorm() {
	mkdir -p $ROOT_DIR/storm/build
	cd $ROOT_DIR/storm/build
	cmake $ROOT_DIR/storm
	make -j$PROCS
	cd $ROOT_DIR
}

function buildStamina() {
	mkdir -p $ROOT_DIR/build
	cd $ROOT_DIR/build
	cmake $ROOT_DIR
	make -j$PROCS
	cd $ROOT_DIR
}

function checkStaminaCloned() {
	# Checks to see if the src/stamina directory exists
	if [ -d "$ROOT_DIR/src/stamina" ];
	then
		msg "STAMINA does not appear to be cloned. Cloning now..."
	else
		git clone $STAMINA_URL stamina-storm
		export ROOT_DIR=$ROOT_DIR/stamina-storm
		msg "STAMINA appears to exist in the current directory"
	fi
}

checkStaminaCloned
mainFnStamina
