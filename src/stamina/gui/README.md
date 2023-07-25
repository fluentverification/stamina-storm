# XSTAMINA (New STAMINA/STORM GUI)

## Compile Instructions

First, run `./CompileUiFiles.sh`. You will need Qt5+ and `uic` (the Qt UI compiler). Then, you can enable building the GUI in the main CMakeLists using `-DBUILD_GUI=On`.

## Dependencies:

- Qt (version 5+)
	+ `uic`: The Qt UI compiler
- KF5 (KDE Frameworks 5)

### Packages by Distro

**Debian and Ubuntu**:
```
sudo apt install build-essential extra-cmake-modules qt5-default
```

**Arch Linux/Manjaro/EndeavorOS/etc.**
```
sudo pacman -S base-devel qt5 kf5
```
