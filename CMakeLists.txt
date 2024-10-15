#
#   module  : CMakeLists.txt
#   version : 1.18
#   date    : 10/15/24
#
cmake_minimum_required(VERSION 3.5)
project(HET VERSION 1.0)
if("${CMAKE_BUILD_TYPE}" STREQUAL "")
  set(CMAKE_BUILD_TYPE "Release")
endif()
if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  option(RUN_TESTS "Run standard tests" ON)
else()
  option(RUN_TESTS "Run standard tests" OFF)
endif()
add_executable(het het.c gc.c)
add_dependencies(het always)
add_custom_target(always
		  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		  COMMAND sh builtin.sh)
if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  add_definitions(-DGC_NOT_DLL -D_CRT_SECURE_NO_DEPRECATE)
else()
  if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O3 --coverage -fprofile-arcs -ftest-coverage") # debug, no optimization
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage") # enabling coverage
  endif()
  target_link_libraries(het m)
endif()
if(RUN_TESTS)
add_custom_target(demo1.txt ALL
		  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		  COMMAND het unit.het >demo1.txt
		  DEPENDS het unit.het)
add_custom_target(comp1 ALL
		  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		  COMMAND diff -w demo1.txt unit.out
		  DEPENDS demo1.txt)
add_custom_target(demo3.txt ALL
		  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		  COMMAND het -d unit.het >demo3.txt
		  DEPENDS het unit.het)
add_custom_target(demo0.txt ALL
		  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		  COMMAND het nonexist.het >demo0.txt)
add_custom_target(demo2.txt ALL
		  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		  COMMAND het tut.het >demo2.txt
		  DEPENDS het tut.het)
add_custom_target(comp2 ALL
		  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		  COMMAND diff -w demo2.txt tut.out
		  DEPENDS demo2.txt)
if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
else()	
add_custom_command(OUTPUT tut.het
		   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		   COMMAND clang -E -x c tut.hetx -I . -o tut.het
		   MAIN_DEPENDENCY tut.hetx lib.hetx
		   DEPENDS tut.hetx lib.hetx
		   COMMENT "Preprocessing tut.hetx"
		   VERBATIM)
endif()	   
endif()
