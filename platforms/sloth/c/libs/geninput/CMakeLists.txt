cmake_minimum_required(VERSION 3.16)
project(SLOTH-GENINPUT LANGUAGES C)

add_library(sloth_geninput_lib STATIC sloth_geninput.c)
target_include_directories(sloth_geninput_lib PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
target_link_libraries(sloth_geninput_lib PUBLIC
  sloth_lib
  sloth_sdl3_lib
)

