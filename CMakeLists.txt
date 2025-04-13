cmake_minimum_required(VERSION 3.23)

project(splash VERSION 1.0
    DESCRIPTION "Splash Task Library"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build" FORCE)
endif()

# Compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
    
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wconversion -Wshadow")
elseif(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /WX")
    set(CMAKE_CXX_FLAGS_DEBUG "/Od /Zi /DEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG")
endif()

include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE  # New in CMake 3.24, but good to have
)
FetchContent_MakeAvailable(googletest)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_library(splash INTERFACE)
target_include_directories(splash INTERFACE ${PROJECT_SOURCE_DIR}/include)

add_executable(pool_tests tests/pool.cpp)
target_link_libraries(pool_tests PRIVATE 
    splash 
    gtest_main
)

add_executable(task_tests tests/task.cpp)
target_link_libraries(task_tests PRIVATE 
    splash 
    gtest_main
)

enable_testing()
add_test(NAME splash_tests COMMAND splash_tests)

if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang"))
    target_compile_options(splash_tests PRIVATE 
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
    )
    target_link_options(splash_tests PRIVATE 
        -fsanitize=address,undefined
    )
endif()