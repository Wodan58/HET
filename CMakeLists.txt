#
#   module  : CMakeLists.txt
#   version : 1.12
#   date    : 06/29/20
#
cmake_minimum_required(VERSION 3.0)
project(HET)
if(WIN32)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 --coverage -fprofile-arcs -ftest-coverage")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()
add_executable(het het.c gc.c)
if(WIN32)
else()
target_link_libraries(het m)
endif()
add_dependencies(het always)
add_custom_target(always
		  COMMAND sh builtin.sh)
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
		   DEPENDS tut.hetx lib.hetx
		   COMMENT "Preprocessing tut.hetx"
		   VERBATIM)
