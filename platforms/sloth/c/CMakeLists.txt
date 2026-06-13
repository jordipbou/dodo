cmake_minimum_required(VERSION 3.16)
project(dodo LANGUAGES C)

include(FetchContent)

FetchContent_Declare(
	sloth
	GIT_REPOSITORY	https://github.com/jordipbou/sloth.git
	GIT_TAG 02d74b9c53b073896d455038f9897d35087b0907
	EXCLUDE_FROM_ALL
)

FetchContent_MakeAvailable(sloth)

add_subdirectory(libs/sdl3)

# Copy 4th files to build directory
file(COPY ${sloth_SOURCE_DIR}/4th/
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/4th)

# Dodo example executable

add_executable(dodo dodo_sample.c)

target_link_libraries(dodo 
	sloth_lib 
	cpnbi_lib 
	sloth_sdl3_lib)
# Set ROOT_PATH to build directory where
# 4th/ scripts can be found
target_compile_definitions(dodo PRIVATE
	ROOT_PATH="${PROJECT_BINARY_DIR}/"
)
