option (MIF_THIRD_PARTY_GIT "[MIF] Download third_party from git" ON)
option (MIF_GITHUB_SOURCE "[MIF] User GitHub as source for download third_party" ON)

set (PROJECT_SERVER_NAME "${PROJECT_LC}_server")
set (PROJECT_CLIENT_NAME "${PROJECT_LC}_client")

add_definitions(-DPYTHON_MODULE_NAME=${PROJECT_LC})

set (CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/MyCMakeScripts)
set (EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

set (MIF_LIB_INSTALL_PATH, ${PROJECT_SOURCE_DIR}/MyCMakeScripts/lib/mif)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -std=c++11")
set (CMAKE_CXX_FLAGS_RELEASE "-O3 -g0 -DNDEBUG")
#set (CMAKE_EXE_LINKER_FLAGS "-static-libstdc++")
set (CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(PythonLibs)
if (NOT DEFINED PYTHON_INCLUDE_DIR)
    message(FATAL_ERROR "[MIF.Examples] Python include dir not found.")
endif()
if (NOT DEFINED PYTHON_INCLUDE_DIR)
    message(FATAL_ERROR "[MIF.Examples] Python libs dir not found.")
endif()
include_directories(${PYTHON_INCLUDE_DIR})
link_directories (${PYTHON_LIBRARY})

set(LIBRARIES
    mif
    boost_python
    boost_regex
    boost_iostreams
    boost_serialization
    boost_program_options
    boost_log
    boost_filesystem
    boost_thread
    boost_system
    jsoncpp
    event
    pthread
    pq.a
    sqlite3
    z
    rt
)
