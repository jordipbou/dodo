cmake_minimum_required(VERSION 3.16)
project(SLOTH-GENINPUT LANGUAGES C)

include(FetchContent)

FetchContent_Declare(
	sloth
	GIT_REPOSITORY	https://github.com/jordipbou/sloth.git
	GIT_TAG dc802f0e1af8b6eb7c434cb9b83b6b6cc1886727
	EXCLUDE_FROM_ALL
)

FetchContent_MakeAvailable(sloth)

add_library(sloth_geninput_lib STATIC sloth_geninput.c)
target_link_libraries(sloth_geninput_lib PUBLIC sloth_lib)
target_include_directories(sloth_sdl3_lib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>
        $<INSTALL_INTERFACE:>  # for install(EXPORT ...) scenarios
)
