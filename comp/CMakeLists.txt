cmake_minimum_required(VERSION 3.25)

project(comp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithAssert" CACHE STRING "" FORCE)

# Add more diagnostics
add_compile_options(-Wall -Wextra -Wpedantic -Wswitch -Wswitch-enum -Wno-sign-compare)

# Disable diagnostic limit
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-ferror-limit=0)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-fmax-errors=0)
endif ()

# Debug flags
if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_options(-DDEBUG -g -ggdb3 -O0 -fno-omit-frame-pointer)
endif ()

# Preserve NDEBUG in RelWithAssert
if (CMAKE_BUILD_TYPE MATCHES RelWithAssert)
    add_compile_options(-UNDEBUG)
endif ()

# Enable assertions in std (both libstdc++ & libc++) for Debug and RelWithAssert builds
if (CMAKE_BUILD_TYPE MATCHES Debug|RelWithAssert)
    if (NOT (MSVC))
        add_definitions("-D_GLIBCXX_DEBUG")
        add_definitions("-D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_DEBUG")

        include(CheckSourceCompiles)

        check_source_compiles(CXX "
#define _LIBCPP_ENABLE_ASSERTIONS 1
#include <iostream>
int main() { std::cout << \"Hello, world!\" << std::endl; }
" OK_WITH_OLD_LIBCPP_FLAG)
        if (OK_WITH_OLD_LIBCPP_FLAG)
            add_definitions("-D_LIBCPP_ENABLE_ASSERTIONS=1")
        endif ()
    endif ()
endif ()

# Enable sanitizers for Debug builds
include(cmake/sanitizers.cmake)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_sanitizers()
endif ()

# Enable LTO for non-Debug builds (if supported)
include(CheckIPOSupported)
check_ipo_supported(RESULT lto_supported OUTPUT error)
if (NOT (lto_supported))
    message(STATUS "LTO not supported: <${error}>")
elseif (CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "LTO supported, but not enabled (Debug build)")
else ()
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    message(STATUS "LTO enabled")
endif ()

# Dependencies
find_package(glog 0.6 REQUIRED)

add_library(bytefile src/bytefile.cpp)
target_include_directories(bytefile PUBLIC include)
target_link_libraries(bytefile glog::glog)

add_executable(lama-rv src/lama_rv.cpp)
target_include_directories(lama-rv PUBLIC include)
target_link_libraries(lama-rv bytefile glog::glog)

add_executable(disasm src/disasm.cpp src/bytefile.cpp)
target_include_directories(disasm PUBLIC include)
target_link_libraries(disasm bytefile glog::glog)
