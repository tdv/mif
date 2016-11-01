cmake_minimum_required(VERSION 2.8)

project(ProtoStack)
set(PROJECT ${PROJECT_NAME})
string(TOLOWER "${PROJECT}" PROJECT_LC)

set(PROJECT_SERVER_NAME "${PROJECT_LC}_server")
set(PROJECT_CLIENT_NAME "${PROJECT_LC}_client")

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/MyCMakeScripts)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -std=c++11")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -g0 -DNDEBUG")
set( CMAKE_EXE_LINKER_FLAGS "-static-libstdc++")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(Boost_USE_STATIC_LIBS ON)  
set(Boost_USE_MULTITHREADED ON)

find_library(MIF_LIBRARY mif)
find_package(Boost COMPONENTS log log_setup thread date_time filesystem system serialization iostreams REQUIRED)
find_package(ZLIB REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${Mif_INCLUDE_DIRS})

include_directories(${CMAKE_SOURCE_DIR}/common/include)

set(COMMON_HEADERS
    ${COMMON_HEADERS}
    ${CMAKE_SOURCE_DIR}/common/include
)

set(SERVER_HEADERS
    ${SERVER_HEADERS}
    ${COMMON_HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/server/src/service.h
)

set(SERVER_SOURCES
    ${SERVER_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/server/src/service.cpp
)

set(CLIENT_HEADERS
    ${SERVER_HEADERS}
    ${COMMON_HEADERS}
)

set(CLIENT_SOURCES
    ${CLIENT_SOURCES}
)

set(LIBRARIES
    ${LIBRARIES}
    ${MIF_LIBRARY}
    ${ZLIB_LIBRARIES}
    ${Boost_LIBRARIES}
    pthread
    z
)

add_executable(${PROJECT_SERVER_NAME} ${SERVER_HEADERS} ${SERVER_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/server/src/main.cpp)
target_link_libraries(${PROJECT_SERVER_NAME} ${LIBRARIES})

add_executable(${PROJECT_CLIENT_NAME} ${CLIENT_HEADERS} ${CLIENT_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/client/src/main.cpp)
target_link_libraries(${PROJECT_CLIENT_NAME} ${LIBRARIES})
