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
	src/dodo.c
	deps/stcp/src/stcp.c deps/stcp/src/socket.c)
target_include_directories (dodo PRIVATE src deps/stcp/include)
set_property (TARGET dodo PROPERTY C_STANDARD 99)
