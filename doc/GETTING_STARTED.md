# Getting Started With the Code

File created by Josh Jeppson (ifndefJOSH) on 8/20/2021 for the purpose of documenting how `stamina-cplusplus` works.

## Entry point

The `main()` function is, as you would guess, in `main.cpp`. Arguments are parsed by GNU's `<argparse.h>` library, and the data for that library is stored in `StaminaArgParse.h`. From the main function, an instance of the `stamina::Stamina` class is instantiated, and the `run()` method of that class is invoked.