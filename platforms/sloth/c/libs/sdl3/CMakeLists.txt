cmake_minimum_required(VERSION 3.16)
project(SLOTH-SDL3 LANGUAGES C)

include(FetchContent)

FetchContent_Declare(
	sloth
	GIT_REPOSITORY	https://github.com/jordipbou/sloth.git
	GIT_TAG 96c21dbef69f7fd7d0e139df13569f3067c08443
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

# sloth_sdl3 library

add_library(sloth_sdl3_lib STATIC sloth_sdl3.c)

find_package(Threads REQUIRED)
target_link_libraries(sloth_sdl3_lib PUBLIC sloth_lib SDL3-static Threads::Threads)

target_include_directories(sloth_sdl3_lib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>
				$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/SDL/include>
        $<INSTALL_INTERFACE:>  # for install(EXPORT ...) scenarios
)

# clear example

add_executable(clear clear.c)
target_link_libraries(clear sloth_sdl3_lib)
