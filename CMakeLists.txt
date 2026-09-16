cmake_minimum_required(VERSION 3.20)

#1. Project Name and Landguage
project(ModernCppMemoryManagement LANGUAGES CXX)

#2. Enforce Modern C++20 Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS OFF)

#3 compiler warning and optimization flags
if (MSVC)
    # windows msvc compiler flags
    add_compile_options(/W4 /permissive-)
    # cant have O2 for testing
    # add_compile_options(/W4 /O2 /permissive-)
else()
    # gcc/ clang compiler flags
    add_compile_options(-Wall -Wextra -Wpedantic -O3)
endif()

#4. expost the "include/" directory so any file can do #include <any thing>
include_directories(include)

#5 enable testing
enable_testing()
add_subdirectory(tests)

# 6. Benchmarks
add_subdirectory(benchmarks)


message(STATUS "Build configure for ${PROJECT_NAME} using C++20")
