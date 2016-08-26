cmake_minimum_required (VERSION 2.6)

project (Test1)
SET (CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/MyCMakeScripts)
#set (SOURCES "${CMAKE_SOURCE_DIR}/src")
set (EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

#set (SOURCE
#	${CMAKE_CURRENT_SOURCE_DIR}/src/*
#)

set(CMAKE_CXX_FLAGS "-Wall -O3 -g0 -std=c++11")

find_package( Boost REQUIRED COMPONENTS system serialization thread)
include_directories( ${Boost_INCLUDE_DIRS} )
include_directories(include)

add_executable (${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
