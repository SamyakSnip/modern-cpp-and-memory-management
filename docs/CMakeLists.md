## 1. Why CMake exists and why we use it

In C++, unlike other languages there is no single build in package manager or compiler command
* on linux g++ or clang++ with Makefiles of Ninja
* on windows MSVC via visual studio .sln/.vcxproj 
* on macOS, Xcode or Clang
if we dont use meta-build system like CMake we would have to write build scrips to every OS. **CMakeList.txt** acts as a rule book and it generates the correct native build files.

## 2. How this root file connects to the rest of the project

This root CMakeLists.txt is the master orchestrator:
* **Global Standard** : enforces c++20 
* **Compiler flags** : detects the os and cofigers compiler wornings
* **Include Paths** : exposed th einclude? folder so we direcly write *#include* "file.hpp" insted of the path */../../include/file.cpp*


```bash
#-------------------------------------------------------------------
cmake_minimum_required(VERSION 3.20)
# sets the minimum required version of cmake


#1. Project Name and Landguage---------------------------------------
project(ModernCppMemoryManagement LANGUAGES CXX)
# declears the project internal name as ModernCppMemoryManagement
# explicitly tess cmake that this project is stricly cpp only CXX this prevents wasting time to search c compilers that we never used in our project





#2. Enforce Modern C++20 Standard---------------------------------

set(CMAKE_CXX_STANDARD 20)
# tells cmake that his project relies heavely on C++20 features 
set(CMAKE_CXX_STANDARD_REQUIRED YES)
# by defauld if there is no cpp20 support compiler decays back to cpp17 or cpp14 this help prevent that and stops building
set(CMAKE_CXX_EXTENSIONS OFF)
#disable compiler-specific dialect so the code can run on all OS 






#3 compiler warning and optimization flags-----------------------------
if (MSVC)
    # windows msvc compiler flags
    add_compile_options(/W4 /permissive-)
    # cant have O2 for testing
    # add_compile_options(/W4 /O2 /permissive-)
else()
    # gcc/ clang compiler flags
    add_compile_options(-Wall -Wextra -Wpedantic -O3)
endif()




#4. expost the "include/" directory so any file can do #include <any thing>-----
include_directories(include)





#5 enable testing----------------------------------------------------------------
enable_testing()
#  (enable_testing()): Activates CMake's built-in test runner, CTest. This must be called in the root CMake file before adding any test subdirectories. Without this line, add_test(...) calls in subdirectories will be silently ignored.
add_subdirectory(tests)
#Tells CMake to jump into the tests/ folder and execute tests/CMakeLists.txt, which configures and registers the unit tests.





# 6. Benchmarks-------------------------------------------------------------------------
add_subdirectory(benchmarks)
#ells CMake to enter the benchmarks/ directory and execute benchmarks/CMakeLists.txt, compiling the nanosecond microbenchmark executable.


message(STATUS "Build configure for ${PROJECT_NAME} using C++20")
#Prints a terminal message when someone runs cmake -B build


```
