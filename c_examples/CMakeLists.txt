cmake_minimum_required (VERSION 3.8)
project(dodo_examples LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

add_executable (
	recursive_fibonacci 
	recursive_fibonacci.c)

add_executable(
	tail_recursive_fibonacci	
	tail_recursive_fibonacci.c)
