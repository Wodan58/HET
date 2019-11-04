#
#   module  : CMakeLists.txt
#   version : 1.9
#   date    : 11/01/19
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
add_dependencies(het always)
add_custom_target(always
		  COMMAND sh builtin.sh
		 )
add_custom_target(demo1.txt ALL
		  COMMAND het unit.het >demo1.txt
		  DEPENDS het unit.het)
add_custom_target(comp1 ALL
		  COMMAND diff -w demo1.txt unit.out
		  DEPENDS demo1.txt)
add_custom_target(demo3.txt ALL
		  COMMAND het -d unit.het >demo3.txt
		  DEPENDS het unit.het)
add_custom_target(demo0.txt ALL
		  COMMAND het nonexist.het >demo0.txt)
add_custom_target(demo2.txt ALL
		  COMMAND het tut.het >demo2.txt
		  DEPENDS het tut.het)
add_custom_target(comp2 ALL
		  COMMAND diff -w demo2.txt tut.out
		  DEPENDS demo2.txt)
add_custom_command(OUTPUT tut.het
		   COMMAND clang -E -x c tut.hetx -I . -o tut.het
		   MAIN_DEPENDENCY tut.hetx lib.hetx
		   COMMENT "Preprocessing tut.hetx"
		   VERBATIM
		  )
