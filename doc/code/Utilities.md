# Utilities

Most of these exist in namespace `stamina::util`

## ModelModify

- This class loads the model and creates modified properties
- Loads model via `readModel()` (must set model name first)
- Load properties via `createPropertiesList()` (must set properties file name first, and also have loaded model)
- Create P<sub>min</sub> and P<sub>max</sub> using the `modifyProperty()` function, given CSL property P.
- You probably don't need to modify this class, except maybe eventually change the name

## StateIndexArray

- This is a linear resizable (expanding-only) array which holds `ProbabilityState`s along an index of `StateType` (generally `uint32_t`)
- There is one of these instantiated in `StaminaModelBuilder`
- Most important method: `get()`. Gets a `ProbabilityState`

## StateMemoryPool

- A simple allocate-only memory pool which allocates `ProbabilityState`s on the fly.
- ***Why?*** If we were going to call `new ProbabilityState()` everytime we wanted a, well, *new ProbabilityState*, we would be basically calling `malloc()` every time we found a new state. In Java, this may be okay, since Java has a built-in memory pool, but in C++ this is not efficient. So, we allocate a bunch of memory at the beginning and continuously use that.
- Most important method: `allocate()`
