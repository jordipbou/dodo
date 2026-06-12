cmake_minimum_required(VERSION 3.16)
project(dodo LANGUAGES C)

#include(cmake/dependencies.cmake)
include(FetchContent)

FetchContent_Declare(
	sloth
	GIT_REPOSITORY	https://github.com/jordipbou/sloth.git
	GIT_TAG 89635760920cf54d77762d44af38aaa3fd73493d 
	EXCLUDE_FROM_ALL
)

FetchContent_Declare(
	SDL3
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
	GIT_TAG 72771ac0b41812065acda4b5c782000293d77ef5
	EXCLUDE_FROM_ALL
)

# Ensure SDL builds as a static library 
set(SDL_STATIC ON CACHE BOOL "" FORCE)

# Fetches all declared deps in one call
FetchContent_MakeAvailable(sloth SDL3)

# Copy 4th files to build directory
file(COPY ${sloth_SOURCE_DIR}/4th/
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/4th)

# Dodo example executable

add_executable(dodo dodo_sample.c)

target_link_libraries(dodo 
	sloth_lib 
	cpnbi_lib 
	SDL3::SDL3)
# Set ROOT_PATH to build directory where
# 4th/ scripts can be found
target_compile_definitions(dodo PRIVATE
	ROOT_PATH="${PROJECT_BINARY_DIR}/"
)
