cmake_minimum_required(VERSION 3.1)

project(YAM)

option(USE_CLANG_TIDY "Run clang-tidy on build" ON)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -fno-strict-aliasing -Wcomments -Wall -Wextra -Wno-potentially-evaluated-expression -fno-omit-frame-pointer")

include_directories("include/")

add_subdirectory(src)

add_subdirectory(example)

enable_testing()

add_subdirectory(tests)
