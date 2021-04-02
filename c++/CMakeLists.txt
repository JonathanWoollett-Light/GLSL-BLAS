# CMake version
cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_COMPILER "/usr/bin/g++-10")

# Project name variable
set(This Example2)

# Sets project name and language `CXX`=C++
project(${This} CXX)

# Sets C++ version
set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan) # Finds Vulkan
include_directories(${Vulkan_INCLUDE_DIR}) # Adds Vulkan

enable_testing() # Sets unit tests
add_subdirectory(googletest) # Adds googletest

# Sets headers and soruce files
set(Headers
    Example.hpp
)
set(Sources
    Example.cpp
)
# Adds library
add_library(${This} STATIC ${Sources} ${Headers})

# Adds test subdirectory
add_subdirectory(test)

# Links Vulkan
target_link_libraries(${This} ${Vulkan_LIBRARY})