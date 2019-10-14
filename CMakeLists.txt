#
#   module  : cmakelists.txt
#   version : 1.7
#   date    : 10/14/19
#
cmake_minimum_required(VERSION 3.0)
project(HET)
add_definitions(-DMALLOC_DEBUG)
if(WIN32)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 --coverage -fprofile-arcs -ftest-coverage")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()
add_executable(het het.c memory.c rmalloc.c)
add_custom_target(demo1.txt ALL
		  COMMAND het unit.het >demo1.txt
		  DEPENDS het unit.het)
add_custom_target(comp1 ALL
		  COMMAND diff -w demo1.txt unit.out
		  DEPENDS demo1.txt)
add_custom_target(demo2.txt ALL
		  COMMAND het -d unit.het >demo2.txt
		  DEPENDS het unit.het)
