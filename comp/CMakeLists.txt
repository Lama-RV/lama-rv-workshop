cmake_minimum_required(VERSION 3.25)

project(comp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithAssert" CACHE STRING "" FORCE)

set(CMAKE_C_FLAGS_RELWITHASSERT "${CMAKE_C_FLAGS_RELEASE} -UNDEBUG" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_RELWITHASSERT "${CMAKE_CXX_FLAGS_RELEASE} -UNDEBUG" CACHE STRING "" FORCE)

set(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG -g -ggdb3 -O0 -fno-omit-frame-pointer")
set(CMAKE_WARN_FLAGS -Wall -Wextra -Werror -pedantic-errors)

if(CMAKE_BUILD_TYPE MATCHES Debug)
    message(STATUS "Debug mode ON")
    set(CMAKE_ASAN_FLAGS -fsanitize=address -fsanitize=undefined -DADDRESS_SANITIZER)
endif(CMAKE_BUILD_TYPE MATCHES Debug)

set(SOURCE_LIST 
    src/bytefile.cpp
    src/lama_rv.cpp)

add_executable(lama-rv ${SOURCE_LIST})
target_include_directories(lama-rv PUBLIC include)

add_executable(disasm src/disasm.cpp src/bytefile.cpp)
target_include_directories(disasm PUBLIC include)
