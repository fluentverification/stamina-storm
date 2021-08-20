# Code Style Guide

Since the JAVA version appeared to have no consistency in coding style, I figured I would add a style guide so we can keep the code fairly consistent here.

## Prototyping classes and functions

In the header file (`.h` and *not* `.hpp` or `.hh`), all STAMINA related objects should be within `namespace stamina`. Within the `.cpp` files, rather than adding `namespace stamina { /* code */ }`, at the beginning of the file, we should add the line `using namespace stamina;`.

### Function prototypes should look as follows:
Either (A)
```
return_type functionName(
    param_type param1
    , param_type param2
    , param_type param3
);

```
Or, if the return type is particularly long:
```
some_namespace::some_namespace::some_namespace::return_type
functionName(
    param_type param1
    , param_type param2
    , param_type param3
);

```

### Function implementations should look as follows

```
return_type
ClassName::functionName(
    param_type param1
    , param_type param2
    , param_type param3
) {
    /* Code here */
}

```
OR (for short lists of params)
```
return_type
ClassName::functionName(param_type param1, param_type param2) {
    /* Code here */
}

```
## Brackets and Indentation

For loops, if statements, class definitions and namespaces, the following bracket style will be used:
```
namespace stamina {
    class SomeClass {
    public:
        SomeClass();
    private:

    protected
        /* Data Members */
        int data;
        int data2;
    };
}
```
```
void functionName() {
    while (condition) {
        if (condition) {
            /* Code */
        }
    }
}
```