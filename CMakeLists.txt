#
#   module  : CMakeLists.txt
#   version : 1.1
#   date    : 09/23/19
#
cmake_minimum_required(VERSION 3.0)
project(HET)
add_executable(het het.c memory.c rmalloc.c)
add_custom_target(demo1.txt ALL
		  COMMAND het unit.het >demo1.txt
		  DEPENDS het unit.het)
add_custom_target(comp1 ALL
		  COMMAND diff demo1.txt unit.out
		  DEPENDS demo1.txt)
