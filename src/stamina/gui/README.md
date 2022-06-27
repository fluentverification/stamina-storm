# XSTAMINA (New STAMINA/STORM GUI)

## Compile Instructions

First, run `./CompileUiFiles.sh`. You will need Qt5+ and `uic` (the Qt UI compiler). Then do the following to use `cmake`:

```bash
mkdir build
cd build
cmake ..
make -j$(nproc --all)
```