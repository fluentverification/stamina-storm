#!/bin/bash

mkdir tmp
cd tmp
git checkout 7a498e9a53aa8a88d2db61b16583b0e6245ad716
cmake ../../.. && make -j8
# Kill after 7 minutes
mkdir out
seq 1 | xargs -II timeout $((7 * 60)) ./stamina-cplusplus $MODEL_FILE $PROPERTIES_FILE 2>>out/original.txt
git checkout 86f6b0786f9bb9390bf989f498291fc5916cceef
cmake ../../.. && make -j8
# Kill after 7 minutes
seq 1 | xargs -II timeout $((7 * 60)) ./stamina-cplusplus $MODEL_FILE $PROPERTIES_FILE 2>>out/latest.txt
diff original.txt latest.txt

git checkout dev
