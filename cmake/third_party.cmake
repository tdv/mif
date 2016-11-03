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

if (MIF_THIRD_PARTY_GIT)
    if (MIF_GITHUB_SOURCE)
        include (cmake/github_source.cmake)
        foreach (lib ${MIF_THIRD_PARTY_LIBS})
            string (TOUPPER ${lib} LIB_NAME_UP)
            
            set (MIF_LIB_GITHUB_URL ${MIF_${LIB_NAME_UP}_GITHUB_URL})
            set (MIF_LIB_GITHUB_TAG ${MIF_${LIB_NAME_UP}_GITHUB_TAG})
            
            message("Add external project \"${lib}\" from GitHub. Tag: \"${MIF_LIB_GITHUB_TAG}\" Url: \"${MIF_LIB_GITHUB_URL}\"")
            
            set (MIF_LIB_INSTALL_DIR ${LIB_NAME_UP}_INSTALLDIR})
            set (MIF_LIB_CMAKE_CACHE_ARGS MIF_${LIB_NAME_UP}_CMAKE_CACHE_ARGS)

            set (MIF_LIB_INSTALL_DIR ${THITD_PARTY_OUTPUT_PATH}/${lib})
            
            set (MIF_LIB_INCLUDE_DIR ${MIF_LIB_INSTALL_DIR}/include)
            include_directories (SYSTEM ${MIF_LIB_INCLUDE_DIR})
            unset (MIF_LIB_INCLUDE_DIR)
            
            set (MIF_LIB_LIBRARIES_DIR ${MIF_LIB_INSTALL_DIR}/lib)
            link_directories (${MIF_LIB_LIBRARIES_DIR})
            unset (MIF_LIB_LIBRARIES_DIR)

            set (MIF_THIRD_PARTY_PROJECT ${lib}-project)
            set (MIF_THIRD_PARTY_PROJECTS "${MIF_THIRD_PARTY_PROJECTS} ${MIF_THIRD_PARTY_PROJECT}")

            ExternalProject_Add (${MIF_THIRD_PARTY_PROJECT}
                GIT_REPOSITORY ${MIF_LIB_GITHUB_URL}
                GIT_TAG ${MIF_LIB_GITHUB_TAG}
                UPDATE_COMMAND ""
                CMAKE_CACHE_ARGS -DCMAKE_INSTALL_PREFIX:INTERNAL=${MIF_LIB_INSTALL_DIR} 
                    -DCMAKE_CXX_FLAGS:INTERNAL=${CMAKE_CXX_FLAGS} 
                    -DBUILD_SHARED_LIBS:INTERNAL=OFF 
                    ${MIF_LIB_CMAKE_CACHE_ARGS}
            )

            unset (MIF_THIRD_PARTY_PROJECT)

            unset (MIF_LIB_GITHUB_URL)
            unset (MIF_LIB_GITHUB_TAG)
            unset (MIF_LIB_INSTALL_DIR)
            unset (MIF_LIB_CMAKE_CACHE_ARGS)
        endforeach (lib)
    else()
        message(FATAL_ERROR "[MIF] No support of getting third_party from not a github source")
    endif()
else()
    message(FATAL_ERROR "[MIF] No support of getting third_party from not a git source")
endif()

#add_custom_target(mif_fake_targets_for_lib)
#add_dependencies(mif_fake_targets_for_lib ${MIF_THIRD_PARTY_PROJECTS})

