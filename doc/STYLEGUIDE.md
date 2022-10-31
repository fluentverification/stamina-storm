# Code Style Guide

Since the JAVA version appeared to have no consistency in coding style, I figured I would add a style guide so we can keep the code fairly consistent here.

## Prototyping classes and functions

In the header file (`.h` and *not* `.hpp` or `.hh`), all STAMINA related objects should be within `namespace stamina`. Within the `.cpp` files, rather than adding `namespace stamina { /* code */ }`, at the beginning of the file and indenting like we would in the header, DO NOT INDENT THE NAMESPACE DECLARATIONS:

```cpp
namespace stamina {
namespace builder {
namespace threads {

// Your code here

} // namespace threads
} // namespace builder
} // namespace stamina
```

### Function prototypes should look as follows:
Either (A)
```cpp
return_type functionName(
    param_type param1
    , param_type param2
    , param_type param3
);

```
Or, if the return type is particularly long:
```cpp
some_namespace::some_namespace::some_namespace::return_type
functionName(
    param_type param1
    , param_type param2
    , param_type param3
);

```

### Function implementations should look as follows

```cpp
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
```cpp
return_type
ClassName::functionName(param_type param1, param_type param2) {
    /* Code here */
}

```
## Brackets and Indentation

For loops, if statements, class definitions and namespaces, the following bracket style will be used:
```cpp
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
```cpp
void functionName() {
    while (condition) {
        if (condition) {
            /* Code */
        }
    }
}
```
