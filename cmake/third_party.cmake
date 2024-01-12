
include(ExternalProject)
include(cmake/options.cmake)
include(cmake/third_party_paths.cmake)

set (MIF_THIRD_PARTY_LIBS
    ${MIF_THIRD_PARTY_LIBS}
    zlib
)

set (ZLIB_LIBRARIES
    z.a
)

set (BOOST_LIBRARIES
    iostreams
    json
    date_time
    serialization
    program_options
    log
    filesystem
    thread
    system
)

set (LIBPQ_LIBRARIES
    pq.a
)

set (MIF_THIRD_PARTY_PROJECTS "")

macro (mif_add_third_party_project_begin project_name)
    string (TOUPPER ${project_name} MIF_PROJECT_NAME_UP)
    set (MIF_LIB_SOURCE_PATH ${MIF_${MIF_PROJECT_NAME_UP}_LOCAL_PATH})

    set (MIF_LIB_INSTALL_DIR ${MIF_PROJECT_NAME_UP}_INSTALLDIR})
    set (MIF_LIB_CMAKE_ARGS "${MIF_${MIF_PROJECT_NAME_UP}_CMAKE_ARGS}")

    set (MIF_LIB_INSTALL_DIR ${THITD_PARTY_OUTPUT_PATH}/${project_name})

    set (MIF_LIB_INCLUDE_DIR ${MIF_LIB_INSTALL_DIR}/include)
    include_directories (SYSTEM ${MIF_LIB_INCLUDE_DIR})
    unset (MIF_LIB_INCLUDE_DIR)

    set (MIF_LIB_LIBRARIES_DIR ${MIF_LIB_INSTALL_DIR}/lib)
    link_directories (${MIF_LIB_LIBRARIES_DIR})
    unset (MIF_LIB_LIBRARIES_DIR)

    set (MIF_THIRD_PARTY_PROJECT ${project_name}-project)
endmacro()

macro (mif_add_third_party_project_end)
    unset (MIF_THIRD_PARTY_PROJECT)
    unset (MIF_LIB_INSTALL_DIR)
    unset (MIF_LIB_CMAKE_ARGS)
    unset (MIF_LIB_SOURCE_PATH)
    unset (MIF_PROJECT_NAME_UP)
endmacro()

function (mif_add_third_party_project project_name)
    mif_add_third_party_project_begin(${project_name})

    ExternalProject_Add (${MIF_THIRD_PARTY_PROJECT}
        SOURCE_DIR ${MIF_LIB_SOURCE_PATH}
        UPDATE_COMMAND ""
        BUILD_IN_SOURCE 1
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:INTERNAL=${MIF_LIB_INSTALL_DIR} 
            -DCMAKE_CXX_FLAGS:INTERNAL=${CMAKE_CXX_FLAGS} 
            -DCMAKE_C_FLAGS:INTERNAL=${CMAKE_C_FLAGS} 
            ${MIF_LIB_CMAKE_ARGS}
        LOG_DOWNLOAD 1
        LOG_UPDATE 1
        LOG_CONFIGURE 1
        LOG_BUILD 1
        LOG_TEST 1
        LOG_INSTALL 1
    )

    mif_add_third_party_project_end()
endfunction()

function (mif_add_boost_project)
    mif_add_third_party_project_begin("boost")

    foreach (lib ${BOOST_LIBRARIES})
        if (DEFINED MIF_LIB_BOOST_LIB_LIST)
            set (MIF_LIB_BOOST_LIB_LIST "${MIF_LIB_BOOST_LIB_LIST},")
        endif()
        set (MIF_LIB_BOOST_LIB_LIST "${MIF_LIB_BOOST_LIB_LIST}${lib}")
    endforeach()

    ExternalProject_Add(${MIF_THIRD_PARTY_PROJECT}
        SOURCE_DIR ${MIF_LIB_SOURCE_PATH}
        UPDATE_COMMAND ""
        BUILD_IN_SOURCE 1
        CONFIGURE_COMMAND ./bootstrap.sh --prefix=${MIF_LIB_INSTALL_DIR} --with-libraries=${MIF_LIB_BOOST_LIB_LIST} --without-icu --without-icu
        BUILD_COMMAND ./b2 install -j8 --disable-icu --ignore-site-config "cxxflags=-std=${MIF_STD_CXX} -fPIC" link=static threading=multi runtime-link=static
        INSTALL_COMMAND ""
        DEPENDS "zlib-project"
        LOG_DOWNLOAD 1
        LOG_UPDATE 1
        LOG_CONFIGURE 1
        LOG_BUILD 1
        LOG_TEST 1
        LOG_INSTALL 1
    )

    mif_add_third_party_project_end()

    set (BOOST_LIBS_LIST "")
    foreach (boost_lib ${BOOST_LIBRARIES})
        list (APPEND BOOST_LIBS_LIST boost_${boost_lib})
    endforeach()
    unset (BOOST_LIBRARIES)
    set (BOOST_LIBRARIES ${BOOST_LIBS_LIST})
    unset (BOOST_LIBS_LIST)

endfunction()

function (mif_add_libpq_project)
    mif_add_third_party_project_begin("libpq")

    ExternalProject_Add(${MIF_THIRD_PARTY_PROJECT}
        SOURCE_DIR ${MIF_LIB_SOURCE_PATH}
        UPDATE_COMMAND ""
        BUILD_IN_SOURCE 1
        CONFIGURE_COMMAND ${MIF_LIB_SOURCE_PATH}configure --enable-thread-safety --without-readline --prefix=${MIF_LIB_INSTALL_DIR}
        BUILD_COMMAND make install MAKELEVEL=0 -C ${MIF_LIB_SOURCE_PATH}src/interfaces/libpq
        INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory src/include ${MIF_LIB_INSTALL_DIR}/include
        LOG_DOWNLOAD 1
        LOG_UPDATE 1
        LOG_CONFIGURE 1
        LOG_BUILD 1
        LOG_TEST 1
        LOG_INSTALL 1
    )

    mif_add_third_party_project_end()
endfunction()

function (mif_add_third_party_paths lib)
    string (TOUPPER ${lib} LIB_NAME_UP)
    if (NOT DEFINED ${LIB_NAME_UP}_INCLUDE_DIR)
        message(FATAL_ERROR "[MIF] The variable ${LIB_NAME_UP}_INCLUDE_DIR is not defined.")
    else()
        message("[MIF] ${LIB_NAME_UP}_INCLUDE_DIR=${${LIB_NAME_UP}_INCLUDE_DIR}")
        include_directories (SYSTEM ${${LIB_NAME_UP}_INCLUDE_DIR})
    endif()
    if (NOT DEFINED ${LIB_NAME_UP}_LIBRARIES_DIR)
        message(FATAL_ERROR "[MIF] The variable ${LIB_NAME_UP}_LIBRARIES_DIR is not defined.")
    else()
        message("[MIF] ${LIB_NAME_UP}_LIBRARIES_DIR=${${LIB_NAME_UP}_LIBRARIES_DIR}")
        link_directories(${${LIB_NAME_UP}_LIBRARIES_DIR})
    endif()
    unset (LIB_NAME_UP)
endfunction()

if (MIF_NEED_THIRD_PARTY_BUILD)
    foreach (lib ${MIF_THIRD_PARTY_LIBS})
        mif_add_third_party_project(${lib})
    endforeach (lib)
    mif_add_boost_project()
    if (MIF_WITH_POSTGRESQL)
        mif_add_libpq_project()
    endif()
else()
    foreach (lib ${MIF_THIRD_PARTY_LIBS})
        mif_add_third_party_paths(${lib})
    endforeach (lib)
    mif_add_third_party_paths(boost)
    if (MIF_WITH_POSTGRESQL)
        mif_add_third_party_paths(libpq)
    endif()
endif()

foreach (lib ${MIF_THIRD_PARTY_LIBS})
    list (APPEND MIF_THIRD_PARTY_PROJECTS ${lib}-project)
endforeach()

list (APPEND MIF_THIRD_PARTY_PROJECTS boost-project)
if (MIF_WITH_POSTGRESQL)
    list (APPEND MIF_THIRD_PARTY_PROJECTS libpq-project)
endif()
