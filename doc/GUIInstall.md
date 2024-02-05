# How to install xSTAMINA (STAMINA GUI)

I've only ever got xSTAMINA to compile on Linux, but it should properly compile on MacOS as well, and should (in theory) also run on Windows. Although Storm is not officially supported on Windows, it may be possible to attain the dependencies via KDE's `craft` package manager.

## Linux

This is by far the easiest way to build and try xSTAMINA.  All you will need to do is install the proper dependencies:

0. Install dependencies:

**Debian/Ubuntu/Derivative**

```sh
# Dependencies for Storm
sudo apt-get -y install build-essential git cmake libboost-all-dev libcln-dev libgmp-dev libginac-dev automake libglpk-dev libhwloc-dev libz3-dev libxerces-c-dev libeigen3-dev
# Dependencies for xSTAMINA
sudo apt install libkf5xmlgui-dev libkf5textwidgets-dev libkf5kio-dev libkf5texteditor-dev qtbase5-dev qtdeclarative5-dev libqt5svg5-dev libkf5i18n-dev libkf5coreaddons-dev extra-cmake-modules
```

**Arch/Derivative**

```sh
# Dependencies for Storm
sudo pacman -Syu base-devel git cmake libboost-all-dev libcln-dev libgmp-dev libginac-dev automake libglpk-dev libhwloc-dev libz3-dev libxerces-c-dev libeigen3-dev
# Dependencies for xSTAMINA
sudo pacman -S kde-sdk-meta
```

### Install Storm

1. Clone Storm from `https://github.com/moves-rwth/storm`
2. Invoke `cmake`
3. Invoke `make` or `cmake --build` to build Storm (ideally with `-j <Number Threads>` flag so it doesn't take all day).
4. Export your build path to `STORM_DIR` (`export STORM_DIR=$(pwd)`)

### Install STAMINA

5. Clone STAMINA from `https://github.com/fluentverification/stamina-storm`
6. Invoke `cmake -DSTORM_PATH=$STORM_DIR -DBUILD_GUI=On`
7. Invoke `make` or `cmake --build` to build STAMINA.
8. This will build an executable called `xstamina`. This is the GUI.

## MacOS

**Note:** This assumes you have `brew`:

```sh
# Install dependencies for Storm
brew install automake cmake boost gmp glpk hwloc z3 xerces-c
# Install dependencies for xSTAMINA
brew install qt@5 argp-standalone
# Install KF5 (dependency for xSTAMINA)
brew untap kde-mac/kde 2> /dev/null
brew tap kde-mac/kde https://invent.kde.org/packaging/homebrew-kde.git --force-auto-update
"$(brew --repo kde-mac/kde)/tools/do-caveats.sh"
brew install kde-mac/kde/kf5-kwidgetsaddons kde-mac/kde/kf5-xmlgui kde-mac/kde/kf5-kio kde-mac/kde/kf5-ktexteditor
```

### Install Storm

1. Clone Storm from `https://github.com/moves-rwth/storm`
2. Invoke `cmake`
3. Invoke `make` or `cmake --build` to build Storm (ideally with `-j <Number Threads>` flag so it doesn't take all day).
4. Export your build path to `STORM_DIR` (`export STORM_DIR=$(pwd)`)

You also may be able to install Storm directly from Brew using:

```sh
brew install stormchecker
```

Then setting `STORM_DIR` from `which storm` (it just needs to point to `libstorm.so` or `libstorm.*`.

### Install STAMINA

5. Clone STAMINA from `https://github.com/fluentverification/stamina-storm`
6. Invoke `cmake -DSTORM_PATH=$STORM_DIR -DBUILD_GUI=On`
7. Invoke `make` or `cmake --build` to build STAMINA.
8. This will build an executable called `xstamina`. This is the GUI.

## Windows

**WARNING**: You are completely on your own here. I have no idea if these will even work.

### Install KDE's `craft` package manager

0. Install KDE's `craft` package manager: Instructions available [here](https://community.kde.org/Craft).

### Install Storm

**Note:** You *cannot* use MSVC/Visual Studio in order to compile Storm and therefore STAMINA/xSTAMINA as Storm will throw an error. You must install and use MinGW (and potentially cygwin).

1. Install the following software for building:
	- git
	- cmake
	- MinGW

2. Use `craft` to install the following dependencies for Storm:
	- boost
	- cln
	- gmp
	- ginac
	- autoreconf
	- glpk
	- hwloc
	- z3

(I don't know what the package names will be in `craft`, so you'll have to use `craft --search` a lot)

3. Clone Storm from `https://github.com/moves-rwth/storm`
4. Invoke `cmake` using the `-DCMAKE_CXX_COMPILER` and `-DCMAKE_C_COMPILER` variables to your MinGW compiler.
5. Invoke `make` or `cmake --build` to build Storm (ideally with `-j <Number Threads>` flag so it doesn't take all day).
6. Export your build path to `STORM_DIR` (`set STORM_DIR=$(pwd)`)

## Install STAMINA

7. Clone STAMINA from `https://github.com/fluentverification/stamina-storm`
8. Invoke `cmake -DSTORM_PATH=$STORM_DIR -DBUILD_GUI=On`
9. Invoke `make` or `cmake --build` to build STAMINA.
10. This will build an executable called `xstamina`. This is the GUI.
