![stamina-logo](doc/staminaLogo.png)

# STAMINA (C++ Version)

C++ version of `stamina` integrating with the `storm` probabilistic model checker at [https://github.com/moves-rwth/storm]. This version is different from the [https://github.com/fluentverification/stamina ]( Java version), and is **still under development**.

## To compile:
### On Linux and Mac:
```
git clone https://github.com/fluentverification/stamina-cplusplus
cd bin
cmake .. -DSTORM_PATH=<PATH TO STORM DIRECTORY>
make
```

### On Windows:
1. Dual-boot Linux or buy a Mac.
2. Run the commands above.

## To run:
The basic syntax of running `stamina` is as follows:
```
./stamina-cplusplus MODEL_FILE PROPERTIES_FILE [OPTIONS...]
```
The following options are allowed (these are *slightly* different than in the Java version):
```
  -c, --const="C1=VAL,C2=VAL,C3=VAL"
                             Comma separated values for constants
  -C, --cuddMaxMem=memory    Maximum CUDD memory, in the same format as PRISM
                             (default: 1g)
  -e, --export=filename      Export model to a (text) file
  -f, --approxFactor=double  Factor to estimate how far off our reachability
                             predictions will be (default: 2.0)
  -i, --import=filename      Import model to a (text) file
  -k, --kappa=double         Reachability threshold for the first iteration
                             (default: 1.0)
  -M, --maxIterations=int    Maximum iteration for solution (default: 10000)
  -n, --maxApproxCount=int   Maximum number of iterations in the approximation
                             (default 10)
  -p, --property=propname    Specify a certain property to check in a model
                             file that contains many
  -r, --reduceKappa=double   Reduction factor for Reachability Threshold
                             (kappa) during the refinement step (default 2.0)
  -R, --noPropRefine         Do not use property based refinement. If given,
                             the model exploration method will reduce kappa and
                             do property independent definement (default: off)
  -S, --exportPerimeterStates=filename
                             Export perimeter states to a file. Please provide
                             a filename. This will append to the file if it is
                             existing
  -t, --exportTrans=filename Export the list of transitions and actions to a
                             specified file name, or to trans.txt if no file
                             name is specified.
                             Transitions are exported in the format <Source
                             State Index> <Destination State Index> <Action
                             Label>
  -T, --rankTransitions      Rank transitions before expanding (default: false)
                            
  -w, --probWin=double       Probability window between lower and upperbound
                             for termination (default: 1.0e-3)
  -?, --help                 Give this help list
      --usage                Give a short usage message
```

## GUI (Work in Progress)

A (simple) GUI is also being developed in Python/PyQt5. Since I don't want to write PyBind bindings for Stamina, the GUI will just invoke the executable using the `os.system()` call and pipe output to the screen. It's essentially just a way to build a command to pass into the C++ executable.
