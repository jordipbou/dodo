cmake_minimum_required(VERSION 3.16)
project(SLOTH-SYSTEM LANGUAGES C)

add_library(sloth_system_lib STATIC sloth_system.c)
target_include_directories(sloth_system_lib PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
# SDL3 is used for its filesystem enumeration and allocation. Reusing
# sloth_sdl3_lib (which links SDL3 PUBLIC) carries the SDL3 target and
# include directories without resolving the dependency twice.
target_link_libraries(sloth_system_lib PUBLIC
  sloth_lib
  sloth_sdl3_lib
)
