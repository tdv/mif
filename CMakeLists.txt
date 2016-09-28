cmake_minimum_required(VERSION 2.8)

project(Mif)
set(PROJECT ${PROJECT_NAME})
string(TOLOWER "${PROJECT}" PROJECT_LC)

set(PROJECT_VERSION_MAJOR 1)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_PATCH 0)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/MyCMakeScripts)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE INTERNAL "")
endif(NOT CMAKE_BUILD_TYPE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -g0 -DNDEBUG")
set( CMAKE_EXE_LINKER_FLAGS "-static-libstdc++")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)


find_package(Boost REQUIRED COMPONENTS system serialization thread)
include_directories(${Boost_INCLUDE_DIRS} )
include_directories(include)

set(MIF_SOURCES
    ${MIF_SOURCES}
    
    #Common
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/common/thread_pool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/common/uuid_generator.cpp

    #Net
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/net/client.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/net/clients/frame_reader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/net/clients/frame_writer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/net/clients/gzip_decompressor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/net/clients/gzip_compressor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/net/tcp_server.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/net/tcp_clients.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/net/detail/tcp_session.cpp
)

set(MIF_HEADERS
    ${MIF_HEADERS}

#    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/common/thread_pool.h

#    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/net/tcp_server.h
#    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/net/tcp_clients.h

#    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/net/detail/tcp_session.h
#    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/net/detail/tcp_server.h
#    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/net/detail/tcp_clients.h
)



add_library(${PROJECT_LC} STATIC ${MIF_SOURCES})
target_link_libraries(${PROJECT_LC} PUBLIC ${Boost_LIBRARIES})

#https://cmake.org/Wiki/CMake/Tutorials/How_to_create_a_ProjectConfig.cmake_file

install (TARGETS ${PROJECT_LC} EXPORT ${PROJECT_LC}
         ARCHIVE DESTINATION lib
         LIBRARY DESTINATION lib
         RUNTIME DESTINATION bin)

install(DIRECTORY include/${PROJECT_LC} DESTINATION include)
