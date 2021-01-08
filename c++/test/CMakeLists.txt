# CMake version
cmake_minimum_required (VERSION 3.8)

# Project name variable
set(This ExampleTests)

# Sets source files
set(Sources
    ExampleTests.cpp
)

# Adds executable
add_executable(${This} ${Sources})

# Adds dependencies
target_link_libraries(${This} PUBLIC
    gtest_main
    Example2
)

add_test(
    NAME ${This}
    COMMAND ${This}
)