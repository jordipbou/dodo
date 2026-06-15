cmake_minimum_required(VERSION 3.16)
project(dodo LANGUAGES C)

if (WIN32)
	add_compile_definitions(	
		_CRT_SECURE_NO_WARNINGS 
		_CRT_NONSTDC_NO_DEPRECATE)
endif()

include(FetchContent)

FetchContent_Declare(
	sloth
	GIT_REPOSITORY	https://github.com/jordipbou/sloth.git
	GIT_TAG 96c21dbef69f7fd7d0e139df13569f3067c08443
	EXCLUDE_FROM_ALL
)

FetchContent_MakeAvailable(sloth)

add_subdirectory(libs/sdl3)

# Copy 4th files to build directory
file(COPY ${sloth_SOURCE_DIR}/4th/
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/4th)

# Dodo example executable

add_executable(dodo dodo.c)

target_link_libraries(dodo 
	sloth_lib 
	cpnbi_lib 
	sloth_sdl3_lib)
# Set ROOT_PATH to build directory where
# 4th/ scripts can be found
target_compile_definitions(dodo PRIVATE
	ROOT_PATH="${PROJECT_BINARY_DIR}/"
)
