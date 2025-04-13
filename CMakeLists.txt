cmake_minimum_required(VERSION 3.10)

project(splash VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Specify include directories.
include_directories(${PROJECT_SOURCE_DIR}/include)

# Add the executable.
add_executable(splash src/task.cpp)