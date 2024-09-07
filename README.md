# MIXAL Compiler

This project implements a MIXAL compiler that translates a simple programming language into MIXAL code. It was developed as part of a course in the 4th semester of the Computer Science Department at Aristotle University of Thessaloniki (ΑΠΘ).

# Overview

The compiler takes a simple source code as input and generates MIXAL code that can be executed on a MIX emulator. The project demonstrates fundamental principles of compilers, including syntax analysis, semantic analysis, and code generation.

# Dependencies

The project relies on the following tools:

    * Flex: Used for lexical analysis (tokenization).
    * Bison: Used for parsing (syntax analysis).
    * Make: Used to manage the build process and compile the project.

Make sure you have these dependencies installed on your system before building the project.

Install Dependencies (Ubuntu/Debian)

```
sudo apt-get install flex bison make
```

Install Dependencies (MacOS - Homebrew)
```
brew install flex bison make
```

# Build Instructions

Once you have all the dependencies installed, you can build the project by running the following command:
```
make test
```
This will build the compiler and generate the MIXAL output file.

Clean Up

To remove all files generated from the build process, use the following command:
```
make clean

```
## License

Under MIT License