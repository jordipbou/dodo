cmake_minimum_required(VERSION 3.16)
project(SLOTH-SDL3 LANGUAGES C)

include(FetchContent)

find_package(SDL3 QUIET)
if(NOT SDL3_FOUND)
  FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG 72771ac0b41812065acda4b5c782000293d77ef5
    EXCLUDE_FROM_ALL
  )
  set(SDL_STATIC ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(SDL3)
endif()

add_library(sloth_sdl3_lib STATIC sloth_sdl3.c)
target_include_directories(sloth_sdl3_lib PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
find_package(Threads REQUIRED)
if(SDL3_FOUND)
  target_link_libraries(sloth_sdl3_lib PUBLIC sloth_lib dodo_common SDL3::SDL3 Threads::Threads)
else()
  target_link_libraries(sloth_sdl3_lib PUBLIC sloth_lib dodo_common SDL3-static Threads::Threads)
endif()
