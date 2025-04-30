# MythoECS
An Entity Component System Framework Based On C++20 And Cmake.

## Prerequisites

To build MythoECS, we need to check the following tools.

### Windows 10/11

- Visual Studio 2022 (or more recent)
- Cmake 3.27 (or more recent)
- Git 2.1 (or more recent)

### Ubuntu 20.04

- Compiler Support C++20 (gcc 9.4/clang 10.0 or more recent)
- Cmake 3.27 (or more recent)
- Git 2.1 (or more recent)

## How To Build With Test

We use cmake to build project, and add some unit tests to check it.

**Notise**: if you do not wanna the unit tests, just change `-DBUILD_TESTS=ON` to `-DBUILD_TESTS=OFF`.

### Build On Windows

Open the project directory by terminal, and create a build directory by the following command:

`mkdir build`

Use cmake tools to build project files:

`cmake -S . -B build -DBUILD_TESTS=ON`

Open the solution in the build directory and build it manually by Visual Studio.

### Build On Linux

Open the project directory by terminal, and create a build directory by the following command:

`mkdir build`

Use cmake tools to build project files:

`cmake -S . -B build -DBUILD_TESTS=ON`

Compile the project:

`cmake --build build`

Run tests:

`ctest --test-dir build`

If wanna tests output, try

`ctest --test-dir build --verbose`