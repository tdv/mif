include(ExternalProject)

include (cmake/options.cmake)

set (MIF_THIRD_PARTY_LIBS
    ${MIF_THIRD_PARTY_LIBS}
    zlib
    jsoncpp
    event
)

set (JSONCPP_LIBRARIES
    jsoncpp
)

set (ZLIB_LIBRARIES
    z.a
)

set (BOOST_LIBRARIES
    iostreams
    serialization
    program_options
    log
    filesystem
    thread
    system
)

set (EVENT_LIBRARIES
    event
    event_core
    event_extra
)

set (MIF_ZLIB_CMAKE_ARGS
)

set (MIF_BOOST_CMAKE_ARGS
)

set (MIF_JSONCPP_CMAKE_ARGS "-DJSONCPP_WITH_TESTS=OFF"
    "-DJSONCPP_WITH_POST_BUILD_UNITTEST=OFF"
    "-DJSONCPP_WITH_PKGCONFIG_SUPPORT=OFF"
    "-DJSONCPP_WITH_CMAKE_PACKAGE=OFF"
    "-DBUILD_SHARED_LIBS=OFF"
    "-DBUILD_STATIC_LIBS=ON"
)

set (MIF_EVENT_CMAKE_ARGS "-DEVENT__BUILD_SHARED_LIBRARIES=OFF"
    "-DEVENT__DISABLE_TESTS=ON"
    "-DEVENT__DISABLE_REGRESS=ON"
    "-DEVENT__DISABLE_SAMPLES=ON"
    "-DEVENT__DISABLE_BENCHMARK=ON"
)

set (MIF_THIRD_PARTY_PROJECTS "")

function (mif_add_third_party_project
        from_git
        project_name
        project_name_upper
        source
        tag
    )
    set (MIF_LIB_INSTALL_DIR ${project_name_upper}_INSTALLDIR})
    set (MIF_LIB_CMAKE_ARGS "${MIF_${project_name_upper}_CMAKE_ARGS}")

    set (MIF_LIB_INSTALL_DIR ${THITD_PARTY_OUTPUT_PATH}/${project_name})

    set (MIF_LIB_INCLUDE_DIR ${MIF_LIB_INSTALL_DIR}/include)
    include_directories (SYSTEM ${MIF_LIB_INCLUDE_DIR})
    unset (MIF_LIB_INCLUDE_DIR)

    set (MIF_LIB_LIBRARIES_DIR ${MIF_LIB_INSTALL_DIR}/lib)
    link_directories (${MIF_LIB_LIBRARIES_DIR})
    unset (MIF_LIB_LIBRARIES_DIR)

    set (MIF_THIRD_PARTY_PROJECT ${project_name}-project)

    if (${from_git})
        ExternalProject_Add (${MIF_THIRD_PARTY_PROJECT}
            GIT_REPOSITORY ${source}
            GIT_TAG ${tag}
            UPDATE_COMMAND ""
            CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:INTERNAL=${MIF_LIB_INSTALL_DIR} 
                -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS} 
                -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS} 
                ${MIF_LIB_CMAKE_ARGS}
        )
    else()
        ExternalProject_Add (${MIF_THIRD_PARTY_PROJECT}
            SOURCE_DIR ${source}
            CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:INTERNAL=${MIF_LIB_INSTALL_DIR} 
                -DCMAKE_CXX_FLAGS:INTERNAL=${CMAKE_CXX_FLAGS} 
                -DCMAKE_C_FLAGS:INTERNAL=${CMAKE_C_FLAGS} 
                ${MIF_LIB_CMAKE_ARGS}
        )
    endif()

    unset (MIF_THIRD_PARTY_PROJECT)

    unset (MIF_LIB_INSTALL_DIR)
    unset (MIF_LIB_CMAKE_ARGS)
endfunction()

function (mif_add_local_third_party_project project_name)
    string (TOUPPER ${project_name} LIB_NAME_UP)
            
    set (MIF_LIB_LOCAL_PATH ${MIF_${LIB_NAME_UP}_LOCAL_PATH})

    mif_add_third_party_project(FALSE ${project_name} ${LIB_NAME_UP} ${MIF_LIB_LOCAL_PATH} "")

    unset (MIF_LIB_LOCAL_PATH)
endfunction()

function (mif_add_git_third_party_project project_name)
    string (TOUPPER ${project_name} LIB_NAME_UP)
            
    set (MIF_LIB_GITHUB_URL ${MIF_${LIB_NAME_UP}_GITHUB_URL})
    set (MIF_LIB_GITHUB_TAG ${MIF_${LIB_NAME_UP}_GITHUB_TAG})

    mif_add_third_party_project(TRUE ${project_name} ${LIB_NAME_UP} ${MIF_LIB_GITHUB_URL} ${MIF_LIB_GITHUB_TAG})

    unset (MIF_LIB_GITHUB_URL)
    unset (MIF_LIB_GITHUB_TAG)
endfunction()

function (mif_add_boost_project from_git)
    set (BOSST_INSTALL_DIR ${THITD_PARTY_OUTPUT_PATH}/boost)
    set (BOOST_INCLUDE_DIR ${BOSST_INSTALL_DIR}/include)
    set (BOOST_LIBRARIES_DIR ${BOSST_INSTALL_DIR}/lib)
    include_directories (SYSTEM ${BOOST_INCLUDE_DIR})
    link_directories(${BOOST_LIBRARIES_DIR})
    foreach (lib ${BOOST_LIBRARIES})
        if (DEFINED MIF_LIB_BOOST_LIB_LIST)
            set (MIF_LIB_BOOST_LIB_LIST "${MIF_LIB_BOOST_LIB_LIST},")
        endif()
        set (MIF_LIB_BOOST_LIB_LIST "${MIF_LIB_BOOST_LIB_LIST}${lib}")
    endforeach()
    if (from_git)
        ExternalProject_Add(boost-project
            GIT_REPOSITORY ${MIF_BOOST_GITHUB_URL}
            GIT_TAG ${MIF_BOOST_GITHUB_TAG}
            BUILD_IN_SOURCE 1
            UPDATE_COMMAND ""
            CONFIGURE_COMMAND ./bootstrap.sh --prefix=${BOSST_INSTALL_DIR} --with-libraries=${MIF_LIB_BOOST_LIB_LIST} --without-icu --without-icu
            BUILD_COMMAND ./b2 install -j8 --disable-icu --ignore-site-config "cxxflags=-std=c++11 -fPIC" link=static threading=multi runtime-link=static
            INSTALL_COMMAND ""
        )
    else()
        ExternalProject_Add(boost-project
            SOURCE_DIR ${MIF_BOOST_LOCAL_PATH}
            BUILD_IN_SOURCE 1
            UPDATE_COMMAND ""
            CONFIGURE_COMMAND ./bootstrap.sh --prefix=${BOSST_INSTALL_DIR} --with-libraries=${MIF_LIB_BOOST_LIB_LIST} --without-icu --without-icu
            BUILD_COMMAND ./b2 install -j8 --disable-icu --ignore-site-config "cxxflags=-std=c++11 -fPIC" link=static threading=multi runtime-link=static
            INSTALL_COMMAND ""
        )
    endif()
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
    if (MIF_THIRD_PARTY_GIT)
        if (MIF_GITHUB_SOURCE)
            include (cmake/github_source.cmake)
            foreach (lib ${MIF_THIRD_PARTY_LIBS})
                mif_add_git_third_party_project(${lib})
            endforeach (lib)
            mif_add_boost_project(TRUE)
        else()
            message(FATAL_ERROR "[MIF] No support of getting third_party from not a github source")
        endif()
    else()
        include (cmake/local_source.cmake)
        foreach (lib ${MIF_THIRD_PARTY_LIBS})
            mif_add_local_third_party_project(${lib})
        endforeach (lib)
        mif_add_boost_project(FALSE)
    endif()
else()
    mif_add_third_party_paths(zlib)
    mif_add_third_party_paths(boost)
    mif_add_third_party_paths(jsoncpp)
    mif_add_third_party_paths(event)
endif()

set (BOOST_LIBS_LIST "")
foreach (boost_lib ${BOOST_LIBRARIES})
    list (APPEND BOOST_LIBS_LIST boost_${boost_lib})
endforeach()
unset (BOOST_LIBRARIES)
set (BOOST_LIBRARIES ${BOOST_LIBS_LIST})
unset (BOOST_LIBS_LIST)

foreach (lib ${MIF_THIRD_PARTY_LIBS})
    list (APPEND MIF_THIRD_PARTY_PROJECTS ${lib}-project)
endforeach()

list (APPEND MIF_THIRD_PARTY_PROJECTS boost-project)
