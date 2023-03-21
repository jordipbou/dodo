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

add_executable (
	core 
	src/core.c)

target_include_directories (core PRIVATE src)

add_executable (
	ctests
	src/ctests.c
	deps/Unity/src/unity.c)

target_include_directories (ctests PRIVATE src deps/Unity/src)

add_executable (
	core2 
	src/core2.c)

target_include_directories (core PRIVATE src)

# add_executable (
# 	ctests2
# 	src/ctests2.c
# 	deps/Unity/src/unity.c)
# 
# target_include_directories (ctests2 PRIVATE src deps/Unity/src)

# add_subdirectory(c_examples)
