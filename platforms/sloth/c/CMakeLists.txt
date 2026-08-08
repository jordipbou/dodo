cmake_minimum_required(VERSION 3.16)
project(dodo LANGUAGES C)
if(WIN32)
	add_compile_definitions(_CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_DEPRECATE)
endif()

include(FetchContent)
FetchContent_Declare(
	sloth
	GIT_REPOSITORY https://github.com/jordipbou/sloth
	GIT_TAG da4c86c561d52cccc0d8f0570c8c3e4356cee23b 
	SOURCE_SUBDIR platforms/c
	EXCLUDE_FROM_ALL
)
FetchContent_Declare(
	cpnbi
	GIT_REPOSITORY https://github.com/jordipbou/cpnbi
	GIT_TAG 2d232dfb5a18214e091e4f740604a19c696c9efe
	EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(sloth cpnbi)   # → sloth_lib, cpnbi_lib

add_subdirectory(libs/sdl3)         # → sloth_sdl3_lib (sistema o vendido)
add_subdirectory(libs/geninput)     # → sloth_geninput_lib

file(COPY ${sloth_SOURCE_DIR}/4th/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/4th)

add_executable(dodo dodo.c)
target_link_libraries(dodo
	sloth_lib
	cpnbi_lib
	sloth_sdl3_lib
	sloth_geninput_lib
)
target_compile_definitions(dodo PRIVATE ROOT_PATH="${PROJECT_BINARY_DIR}/")
