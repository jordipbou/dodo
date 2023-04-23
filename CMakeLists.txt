cmake_minimum_required (VERSION 3.8)

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_C_FLAGS "-ansi -pedantic -pg")

project(dodo LANGUAGES C)

set(dodo_VERSION_MAJOR 0)
set(dodo_VERSION_MINOR 1)

add_executable(typed src/typed.c)
target_include_directories (typed PRIVATE src)

add_executable(typed_tests src/typed_tests.c deps/Unity/src/unity.c)
target_include_directories (typed_tests PRIVATE src deps/Unity/src)

#add_executable (
#	dodo 
#	src/dodo.c)
#
#target_include_directories (dodo PRIVATE src)
#
#add_executable (
#	tests
#	src/tests.c
#	deps/Unity/src/unity.c)
#
#target_include_directories (tests PRIVATE src deps/Unity/src)

# add_executable (
# 	core_tests
# 	src/core_tests.c
# 	deps/Unity/src/unity.c)
# 
# target_include_directories (core_tests PRIVATE src deps/Unity/src)

#add_executable (
#	simp_tests
#	src/simp_tests.c
#	deps/Unity/src/unity.c)
#
#target_include_directories (simp_tests PRIVATE src deps/Unity/src)

# add_subdirectory(c_examples)
