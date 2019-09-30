#
#   module  : CMakeLists.txt
#   version : 1.2
#   date    : 09/30/19
#
cmake_minimum_required(VERSION 3.0)
project(HET)
if(WIN32)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()
add_executable(het het.c memory.c rmalloc.c)
add_custom_target(demo1.txt ALL
		  COMMAND het unit.het >demo1.txt
		  DEPENDS het unit.het)
add_custom_target(comp1 ALL
		  COMMAND diff demo1.txt unit.out
		  DEPENDS demo1.txt)
