set (PROJECT_SERVER_NAME "${PROJECT_LC}_server")
set (PROJECT_CLIENT_NAME "${PROJECT_LC}_client")

set (CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/MyCMakeScripts)
set (EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

set (MIF_LIB_INSTALL_PATH, ${PROJECT_SOURCE_DIR}/MyCMakeScripts/lib/mif)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -std=c++11")
set (CMAKE_CXX_FLAGS_RELEASE "-O3 -g0 -DNDEBUG")
#set (CMAKE_EXE_LINKER_FLAGS "-static-libstdc++")
set (CMAKE_POSITION_INDEPENDENT_CODE ON)

set(LIBRARIES
    mif
    boost_iostreams
    boost_date_time
    boost_serialization
    boost_program_options
    boost_log
    boost_filesystem
    boost_thread
    boost_system
    jsoncpp
    event
    pugixml
    pthread
    pq.a
    sqlite3
    z
    rt
)
