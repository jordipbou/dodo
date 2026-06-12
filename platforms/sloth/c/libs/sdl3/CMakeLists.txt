cmake_minimum_required(VERSION 3.16)
project(SLOTH-SDL3 LANGUAGES C)

set(SDL_STATIC ON)
add_subdirectory(SDL EXCLUDE_FROM_ALL)

set(SLOTH_FLOATING_POINT ON)
add_subdirectory(../sloth/platforms/c ${CMAKE_CURRENT_BINARY_DIR}/sloth EXCLUDE_FROM_ALL)

# sloth_sdl3 library

add_library(sloth_sdl3 STATIC sloth_sdl3.c)

find_package(Threads REQUIRED)
target_link_libraries(sloth_sdl3 PUBLIC sloth_lib SDL3-static Threads::Threads)

target_include_directories(sloth_sdl3
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>
				$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/SDL/include>
        $<INSTALL_INTERFACE:>  # for install(EXPORT ...) scenarios
)

# clear example

add_executable(clear clear.c)
target_link_libraries(clear sloth_sdl3)
