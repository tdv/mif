include(ExternalProject)
include (../common/cmake/settings.cmake)

include_directories (SYSTEM ${MIF_LIB_INSTALL}/include)
link_directories (${MIF_LIB_INSTALL}/lib)

ExternalProject_Add (${MIF_LIB_PROJECT}
    SOURCE_DIR ${MIF_LIB_SRC}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:INTERNAL=${MIF_LIB_INSTALL} 
        -DCMAKE_CXX_FLAGS:INTERNAL=${CMAKE_CXX_FLAGS} 
        ${MIF_LIB_CMAKE_ARGS}
)
