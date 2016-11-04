include(ExternalProject)

include (cmake/options.cmake)

set (MIF_THIRD_PARTY_LIBS
    ${MIF_THIRD_PARTY_LIBS}
    zlib
#    boost
    jsoncpp
    event
)

set (JSONCPP_LIBRARIES
    z
)

set (ZLIB_LIBRARIES
    z
)

set (EVENT_LIBRARIES
    event
    event_core
    event_extra
)

set (MIF_ZLIB_CMAKE_CACHE_ARGS
)

set (MIF_BOOST_CMAKE_CACHE_ARGS
)

set (MIF_JSONCPP_CMAKE_CACHE_ARGS
)

set (MIF_EVENT_CMAKE_CACHE_ARGS
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
    set (MIF_LIB_CMAKE_CACHE_ARGS MIF_${project_name_upper}_CMAKE_CACHE_ARGS)

    set (MIF_LIB_INSTALL_DIR ${THITD_PARTY_OUTPUT_PATH}/${project_name})

    set (MIF_LIB_INCLUDE_DIR ${MIF_LIB_INSTALL_DIR}/include)
    include_directories (SYSTEM ${MIF_LIB_INCLUDE_DIR})
    unset (MIF_LIB_INCLUDE_DIR)

    set (MIF_LIB_LIBRARIES_DIR ${MIF_LIB_INSTALL_DIR}/lib)
    link_directories (${MIF_LIB_LIBRARIES_DIR})
    unset (MIF_LIB_LIBRARIES_DIR)

    set (MIF_THIRD_PARTY_PROJECT ${project_name}-project)
    set (MIF_THIRD_PARTY_PROJECTS "${MIF_THIRD_PARTY_PROJECTS} ${MIF_THIRD_PARTY_PROJECT}")

    if (${from_git})
        ExternalProject_Add (${MIF_THIRD_PARTY_PROJECT}
            GIT_REPOSITORY ${source}
            GIT_TAG ${tag}
            UPDATE_COMMAND ""
            CMAKE_CACHE_ARGS -DCMAKE_INSTALL_PREFIX:INTERNAL=${MIF_LIB_INSTALL_DIR} 
                -DCMAKE_CXX_FLAGS:INTERNAL=${CMAKE_CXX_FLAGS} 
                -DBUILD_SHARED_LIBS:INTERNAL=OFF 
                ${MIF_LIB_CMAKE_CACHE_ARGS}
        )
    else()
        ExternalProject_Add (${MIF_THIRD_PARTY_PROJECT}
            SOURCE_DIR ${source}
            CMAKE_CACHE_ARGS -DCMAKE_INSTALL_PREFIX:INTERNAL=${MIF_LIB_INSTALL_DIR} 
                -DCMAKE_CXX_FLAGS:INTERNAL=${CMAKE_CXX_FLAGS} 
                -DBUILD_SHARED_LIBS:INTERNAL=OFF 
                ${MIF_LIB_CMAKE_CACHE_ARGS}
        )
    endif()

    unset (MIF_THIRD_PARTY_PROJECT)

    unset (MIF_LIB_INSTALL_DIR)
    unset (MIF_LIB_CMAKE_CACHE_ARGS)
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

if (MIF_THIRD_PARTY_GIT)
    if (MIF_GITHUB_SOURCE)
        include (cmake/github_source.cmake)
        foreach (lib ${MIF_THIRD_PARTY_LIBS})
            mif_add_git_third_party_project(${lib})
        endforeach (lib)
    else()
        message(FATAL_ERROR "[MIF] No support of getting third_party from not a github source")
    endif()
else()
    include (cmake/local_source.cmake)
    foreach (lib ${MIF_THIRD_PARTY_LIBS})
        mif_add_local_third_party_project(${lib})
    endforeach (lib)
endif()

#add_custom_target(mif_fake_targets_for_lib)
#add_dependencies(mif_fake_targets_for_lib ${MIF_THIRD_PARTY_PROJECTS})

