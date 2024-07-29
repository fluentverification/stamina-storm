# Core of STAMINA

* * *

In the `stamina` namespace:

## Stamina

- Easy class to run STAMINA from. Used by both the CLI and GUI
- Just tell it model and properties file, and invoke `initialize()` and `run()` (`run()` invokes `initialize()` too so you don't even need to call it yourself).
- Get a table of results via `getResultTable()`

* * *

In the `stamina::core` namespace:

## Options

- Where all Options are defined. Static definition, so shared by the entire program
- Must set before running
- Can use `setArgs()` to set arguments from the arg parser used by the CLI

## StaminaMessages

- A number of static functions to print STAMINA-specific messages to the screen, and to the log-viewer in the GUI.
- If compiling with the GUI, please ensure that the GUI sets the appropriate *callbacks* to put text onto its log viewer.
- Just import `StaminaMessages.h` and then use functions.
	+ **Info** (`StaminaMessages::info()`): general log message
	+ **Warning** (`StaminaMessages::warning()`): warnings and stuff the user should be aware of that could negatively affect program running
	+ **Error** (`StaminaMessages::warning()`): Noncritical error messages (if you want STAMINA to terminate please use `errorAndExit()`)
	+ **Error and Exit** (`StaminaMessages::errorAndExit()`): critical errors that mean STAMINA must exit. Can pass in exit code as second parameter.
- These will not print if `--quiet/-q` is included, aka if `Options::quiet` is `true`.

## StateSpaceInformation

- Small class used to get information about a compressed state given the current known information about the state space.
- Must set current model's variable information (using `setVariableInformation()`) before using. This can be obtained from the model object.

## StaminaModelChecker

- Wrapper class which does the model checking until satisfactory for STAMINA
- Instantiates a `StaminaModelBuilder` which it uses to build the transition matrices
- Calls Storm to check the results.
