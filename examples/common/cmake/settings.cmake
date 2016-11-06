set (PROJECT_SERVER_NAME "${PROJECT_LC}_server")
set (PROJECT_CLIENT_NAME "${PROJECT_LC}_client")

set (CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/MyCMakeScripts)
set (EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

set (MIF_LIB_INSTALL_PATH, ${PROJECT_SOURCE_DIR}/MyCMakeScripts/lib/mif)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -std=c++11")
set (CMAKE_CXX_FLAGS_RELEASE "-O3 -g0 -DNDEBUG")
#set (CMAKE_EXE_LINKER_FLAGS "-static-libstdc++")
set (CMAKE_POSITION_INDEPENDENT_CODE ON)

set (Boost_USE_STATIC_LIBS ON)  
set (Boost_USE_MULTITHREADED ON)

set (MIF_LIB_PROJECT "mif-project")
set (MIF_LIB_INSTALL "${CMAKE_SOURCE_DIR}/lib/mif")
set (MIF_LIB_SRC "../../../")

set (THITD_PARTY_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/lib")

set (MIF_LIB_CMAKE_ARGS
    "-DMIF_SHARED_LIBS=OFF"
    "-DMIF_THIRD_PARTY_GIT=OFF"
    "-DTHITD_PARTY_OUTPUT_PATH=${THITD_PARTY_OUTPUT_PATH}"
)

include_directories (SYSTEM ${THITD_PARTY_OUTPUT_PATH}/zlib/include)
link_directories (${THITD_PARTY_OUTPUT_PATH}/zlib/lib)

include_directories (SYSTEM ${THITD_PARTY_OUTPUT_PATH}/boost/include)
link_directories (${THITD_PARTY_OUTPUT_PATH}/boost/lib)

include_directories (SYSTEM ${THITD_PARTY_OUTPUT_PATH}/jsoncpp/include)
link_directories (${THITD_PARTY_OUTPUT_PATH}/jsoncpp/lib)

#include_directories (SYSTEM ${THITD_PARTY_OUTPUT_PATH}/event/include)
#link_directories (${THITD_PARTY_OUTPUT_PATH}/event/lib)

include_directories (SYSTEM ${MIF_LIB_INSTALL_PATH}/mif/include)
link_directories (${MIF_LIB_INSTALL_PATH}/mif/lib)

set(LIBRARIES
    mif
    boost_iostreams
    boost_serialization
    boost_log
    boost_filesystem
    boost_thread
    boost_system
    pthread
    z
)
