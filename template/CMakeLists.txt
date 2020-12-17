cmake_minimum_required(VERSION 2.8)

include(ExternalProject)

project(my_app)
set(PROJECT ${PROJECT_NAME})
string(TOLOWER "${PROJECT}" PROJECT_LC)

set (STD_CXX "c++11")

set (CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/MyCMakeScripts)
set (EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -std=${STD_CXX}")
set (CMAKE_CXX_FLAGS_RELEASE "-O3 -g0 -DNDEBUG")
set (CMAKE_POSITION_INDEPENDENT_CODE ON)

#---------------------------------------------------------

#---------------------Third party common -----------------

set (THITD_PARTY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/third_party)
set (THIRD_PARTY_PROJECTS "")
set (LIBRARIES "")

list (APPEND LIBRARIES "mif")

set (ZLIB_URL "https://github.com/madler/zlib.git")
set (ZLIB_TAG "v1.2.9")

set (BOOST_URL "https://dl.bintray.com/boostorg/release/1.74.0/source/boost_1_74_0.tar.gz")

set (PUGIXML_URL "http://github.com/zeux/pugixml/releases/download/v1.8/pugixml-1.8.tar.gz")

set (LIBPQ_URL "https://github.com/postgres/postgres.git")
set (LIBPQ_TAG "REL9_6_2")

set (MIF_URL "https://github.com/tdv/mif.git")
set (MIF_TAG "master")

#------------------------ ZLib ---------------------------
set (ZLIB_INSTALL_DIR ${THITD_PARTY_OUTPUT_PATH}/zlib)
set (ZLIB_INCLUDE_DIR ${ZLIB_INSTALL_DIR}/include)
set (ZLIB_LIBRARIES_DIR ${ZLIB_INSTALL_DIR}/lib)
include_directories (SYSTEM ${ZLIB_INCLUDE_DIR})
link_directories(${ZLIB_LIBRARIES_DIR})

set (ZLIB_PROJECT "zlib-project")
list (APPEND THIRD_PARTY_PROJECTS ${ZLIB_PROJECT})

ExternalProject_Add (${ZLIB_PROJECT}
            GIT_REPOSITORY ${ZLIB_URL}
            GIT_TAG ${ZLIB_TAG}
            UPDATE_COMMAND ""
            BUILD_IN_SOURCE 1
            CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:INTERNAL=${ZLIB_INSTALL_DIR} 
                -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS} 
                -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS} 
            LOG_DOWNLOAD 1
            LOG_UPDATE 1
            LOG_CONFIGURE 1
            LOG_BUILD 1
            LOG_TEST 1
            LOG_INSTALL 1
        )

list (APPEND LIBRARIES "z.a")
#---------------------------------------------------------

#------------------------ Boost --------------------------
set (BOOST_LIBRARIES
    "iostreams"
    "json"
    "date_time"
    "serialization"
    "program_options"
    "log"
    "filesystem"
    "thread"
    "system"
)

set (BOOST_INSTALL_DIR ${THITD_PARTY_OUTPUT_PATH}/boost)
set (BOOST_INCLUDE_DIR ${BOOST_INSTALL_DIR}/include)
set (BOOST_LIBRARIES_DIR ${BOOST_INSTALL_DIR}/lib)
include_directories (SYSTEM ${BOOST_INCLUDE_DIR})
link_directories(${BOOST_LIBRARIES_DIR})

set (BOOST_PROJECT "boost-project")
list (APPEND THIRD_PARTY_PROJECTS ${BOOST_PROJECT})

foreach (lib ${BOOST_LIBRARIES})
    if (DEFINED BOOST_LIBRARIES_LIST)
        set (BOOST_LIBRARIES_LIST "${BOOST_LIBRARIES_LIST},")
    endif()
    set (BOOST_LIBRARIES_LIST "${BOOST_LIBRARIES_LIST}${lib}")
endforeach()

ExternalProject_Add(${BOOST_PROJECT}
        URL ${BOOST_URL}
        UPDATE_COMMAND ""
        BUILD_IN_SOURCE 1
        CONFIGURE_COMMAND ./bootstrap.sh --prefix=${BOOST_INSTALL_DIR} --with-libraries=${BOOST_LIBRARIES_LIST} --without-icu --without-icu
        DEPENDS ${ZLIB_PROJECT}
        BUILD_COMMAND ./b2 install -j8 --disable-icu --ignore-site-config "cxxflags=-std=${STD_CXX} -fPIC" link=static threading=multi runtime-link=static
        INSTALL_COMMAND ""
        LOG_DOWNLOAD 1
        LOG_UPDATE 1
        LOG_CONFIGURE 1
        LOG_BUILD 1
        LOG_TEST 1
        LOG_INSTALL 1
    )

foreach (lib ${BOOST_LIBRARIES})
    list (APPEND LIBRARIES boost_${lib})
endforeach()
#---------------------------------------------------------

#------------------------ LibPQ --------------------------
set (LIBPQ_INSTALL_DIR ${THITD_PARTY_OUTPUT_PATH}/pq)
set (LIBPQ_INCLUDE_DIR ${LIBPQ_INSTALL_DIR}/include)
set (LIBPQ_LIBRARIES_DIR ${LIBPQ_INSTALL_DIR}/lib)
include_directories (SYSTEM ${LIBPQ_INCLUDE_DIR})
link_directories(${LIBPQ_LIBRARIES_DIR})

set (LIBPQ_PROJECT "pq-project")
list (APPEND THIRD_PARTY_PROJECTS ${LIBPQ_PROJECT})

ExternalProject_Add (${LIBPQ_PROJECT}
            GIT_REPOSITORY ${LIBPQ_URL}
            GIT_TAG ${LIBPQ_TAG}
            UPDATE_COMMAND ""
            BUILD_IN_SOURCE 1
            CONFIGURE_COMMAND ./configure --enable-thread-safety --without-readline --prefix=${LIBPQ_INSTALL_DIR}
            BUILD_IN_SOURCE 1
            BUILD_COMMAND make install -C src/interfaces/libpq
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory src/include ${LIBPQ_INSTALL_DIR}/include
            LOG_DOWNLOAD 1
            LOG_UPDATE 1
            LOG_CONFIGURE 1
            LOG_BUILD 1
            LOG_TEST 1
            LOG_INSTALL 1
        )

list (APPEND LIBRARIES "pq.a")
#---------------------------------------------------------

#------------------------ PugiXml ------------------------
set (PUGIXML_INSTALL_DIR ${THITD_PARTY_OUTPUT_PATH}/pugixml)
set (PUGIXML_INCLUDE_DIR ${PUGIXML_INSTALL_DIR}/include)
set (PUGIXML_LIBRARIES_DIR ${PUGIXML_INSTALL_DIR}/lib)
include_directories (SYSTEM ${PUGIXML_INCLUDE_DIR})
link_directories(${PUGIXML_LIBRARIES_DIR})

set (PUGIXML_PROJECT "pugixml-project")
list (APPEND THIRD_PARTY_PROJECTS ${PUGIXML_PROJECT})

ExternalProject_Add (${PUGIXML_PROJECT}
            URL ${PUGIXML_URL}
            UPDATE_COMMAND ""
            BUILD_IN_SOURCE 1
            CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:INTERNAL=${PUGIXML_INSTALL_DIR} 
                -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS} 
                -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS} 
            LOG_DOWNLOAD 1
            LOG_UPDATE 1
            LOG_CONFIGURE 1
            LOG_BUILD 1
            LOG_TEST 1
            LOG_INSTALL 1
        )

list (APPEND LIBRARIES "pugixml")
#---------------------------------------------------------

#------------------------ MIF ----------------------------
set (MIF_INSTALL_DIR ${THITD_PARTY_OUTPUT_PATH}/mif)
set (MIF_INCLUDE_DIR ${MIF_INSTALL_DIR}/include)
set (MIF_LIBRARIES_DIR ${MIF_INSTALL_DIR}/lib)
include_directories (SYSTEM ${MIF_INCLUDE_DIR})
link_directories(${MIF_LIBRARIES_DIR})

set (MIF_PROJECT "mif-project")
list (APPEND THIRD_PARTY_PROJECTS ${MIF_PROJECT})

ExternalProject_Add (${MIF_PROJECT}
            GIT_REPOSITORY ${MIF_URL}
            GIT_TAG ${MIF_TAG}
            UPDATE_COMMAND ""
            BUILD_IN_SOURCE 1
            CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:INTERNAL=${MIF_INSTALL_DIR} 
                -DMIF_STD_CXX=${STD_CXX}
                -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS} 
                -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS} 
                -DMIF_NEED_THIRD_PARTY_BUILD=OFF
                -DZLIB_INCLUDE_DIR=${ZLIB_INCLUDE_DIR}
                -DZLIB_LIBRARIES_DIR=${ZLIB_LIBRARIES_DIR}
                -DBOOST_INCLUDE_DIR=${BOOST_INCLUDE_DIR}
                -DBOOST_LIBRARIES_DIR=${BOOST_LIBRARIES_DIR}
                -DLIBPQ_INCLUDE_DIR=${LIBPQ_INCLUDE_DIR}
                -DLIBPQ_LIBRARIES_DIR=${LIBPQ_LIBRARIES_DIR}
                -DPUGIXML_INCLUDE_DIR=${PUGIXML_INCLUDE_DIR}
                -DPUGIXML_LIBRARIES_DIR=${PUGIXML_LIBRARIES_DIR}
            DEPENDS ${BOOST_PROJECT} ${PUGIXML_PROJECT} ${LIBPQ_PROJECT}
            BUILD_COMMAND ${MAKE}
            LOG_DOWNLOAD 1
            LOG_UPDATE 1
            LOG_CONFIGURE 1
            LOG_BUILD 1
            LOG_TEST 1
            LOG_INSTALL 1
        )
#---------------------------------------------------------

set(HEADERS
    ${HEADERS}
    ${COMMON_HEADERS}
)

set(SOURCES
    ${SOURCES}
)

set(LIBRARIES
    ${LIBRARIES}
    pthread
    rt
)

include_directories (include)
include_directories (${HEADERS})

add_executable(${PROJECT_LC} ${HEADERS} ${SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)
target_link_libraries(${PROJECT_LC} ${LIBRARIES})
add_dependencies (${PROJECT_LC} ${THIRD_PARTY_PROJECTS})
