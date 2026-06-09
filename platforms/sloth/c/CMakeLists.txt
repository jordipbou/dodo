cmake_minimum_required(VERSION 3.16)
project(dodo LANGUAGES C)

set(SLOTH_ROOT_PATH "${PROJECT_BINARY_DIR}/../../sloth/")

add_subdirectory(sloth/platforms/c EXCLUDE_FROM_ALL)

set(SDL_STATIC ON)
add_subdirectory(SDL EXCLUDE_FROM_ALL)

add_executable(dodo dodo.c)
target_link_libraries(dodo sloth_lib cpnbi_lib SDL3::SDL3)
target_compile_definitions(dodo PRIVATE
	ROOT_PATH="${SLOTH_ROOT_PATH}"
)
