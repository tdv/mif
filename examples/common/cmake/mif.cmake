include(ExternalProject)
include (../common/cmake/settings.cmake)

set (MIF_LIB_PROJECT "mif-project")
set (MIF_LIB_INSTALL "${CMAKE_SOURCE_DIR}/lib/mif")
set (MIF_LIB_SRC "../../../")

set (MIF_LIB_CMAKE_ARGS
    "-DMIF_SHARED_LIBS=OFF"
    "-DMIF_THIRD_PARTY_GIT=${MIF_THIRD_PARTY_GIT}"
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


include_directories (SYSTEM ${MIF_LIB_INSTALL}/include)
link_directories (${MIF_LIB_INSTALL}/lib)

ExternalProject_Add (${MIF_LIB_PROJECT}
    SOURCE_DIR ${MIF_LIB_SRC}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:INTERNAL=${MIF_LIB_INSTALL} 
        -DCMAKE_CXX_FLAGS:INTERNAL=${CMAKE_CXX_FLAGS} 
        ${MIF_LIB_CMAKE_ARGS}
)
