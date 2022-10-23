cmake_minimum_required (VERSION 3.8)
project(dodo LANGUAGES C)

set (dodo_VERSION_MAJOR 0)
set (dodo_VERSION_MINOR 1)

if (NOT CMAKE_BUILD_TYPE)
	set (CMAKE_BUILD_TYPE Release)
endif()

set (CMAKE_C_FLAGS_RELEASE "-O3")
set (CMAKE_EXE_LIMKER_FLAGS "")

add_executable (
	dodo 
	src/dodo.c src/vm.c)
#	deps/stcp/src/stcp.c deps/stcp/src/socket.c)
target_include_directories (dodo PRIVATE src) # deps/stcp/include)
set_property (TARGET dodo PROPERTY C_STANDARD 99)

add_executable (
	tests
	src/tests.c src/vm.c
	deps/Unity/src/unity.c)
target_include_directories (tests PRIVATE src deps/Unity/src)
set_property (TARGET tests PROPERTY C_STANDARD 99)
