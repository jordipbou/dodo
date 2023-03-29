cmake_minimum_required (VERSION 3.8)
project(dodo LANGUAGES C)

set (dodo_VERSION_MAJOR 0)
set (dodo_VERSION_MINOR 1)

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

if (NOT CMAKE_BUILD_TYPE)
	set (CMAKE_BUILD_TYPE Release)
endif()

set (CMAKE_C_FLAGS_RELEASE "-O3")
set (CMAKE_EXE_LIMKER_FLAGS "")

add_executable (
	dodo 
	src/dodo.c)

target_include_directories (dodo PRIVATE src)

add_executable (
	tests
	src/tests.c
	deps/Unity/src/unity.c)

target_include_directories (tests PRIVATE src deps/Unity/src)

add_executable (lists_tests src/lists_tests.c deps/Unity/src/unity.c)
target_include_directories (lists_tests PRIVATE src deps/Unity/src)

add_executable (core_tests src/core_tests.c deps/Unity/src/unity.c)
target_include_directories (core_tests PRIVATE src deps/Unity/src)

add_executable (stack_tests src/stack_tests.c deps/Unity/src/unity.c)
target_include_directories (stack_tests PRIVATE src deps/Unity/src)

add_executable (error_tests src/error_tests.c deps/Unity/src/unity.c)
target_include_directories (error_tests PRIVATE src deps/Unity/src)

add_executable (comparison_tests src/comparison_tests.c deps/Unity/src/unity.c)
target_include_directories (comparison_tests PRIVATE src deps/Unity/src)

add_executable (arithmetic_tests src/arithmetic_tests.c deps/Unity/src/unity.c)
target_include_directories (arithmetic_tests PRIVATE src deps/Unity/src)

# add_subdirectory(c_examples)
