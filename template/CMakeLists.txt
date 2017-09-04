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

set (THITD_PARTY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/third_party)
set (THIRD_PARTY_PROJECTS "")
set (LIBRARIES "")

list (APPEND LIBRARIES "mif")

#------------------------ ZLib ---------------------------
set (ZLIB_INSTALL_DIR ${THITD_PARTY_OUTPUT_PATH}/zlib)
set (ZLIB_INCLUDE_DIR ${ZLIB_INSTALL_DIR}/include)
set (ZLIB_LIBRARIES_DIR ${ZLIB_INSTALL_DIR}/lib)
include_directories (SYSTEM ${ZLIB_INCLUDE_DIR})
link_directories(${ZLIB_LIBRARIES_DIR})

set (ZLIB_PROJECT "zlib-project")
list (APPEND THIRD_PARTY_PROJECTS ${ZLIB_PROJECT})

ExternalProject_Add (${ZLIB_PROJECT}
            GIT_REPOSITORY https://github.com/madler/zlib.git
            GIT_TAG v1.2.8
            UPDATE_COMMAND ""
            BUILD_IN_SOURCE 1
            CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:INTERNAL=${ZLIB_INSTALL_DIR} 
                -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS} 
                -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS} 
        )

list (APPEND LIBRARIES "z.a")
#---------------------------------------------------------

#------------------------ Boost --------------------------
set (BOOST_LIBRARIES
    "iostreams"
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
        URL https://sourceforge.net/projects/boost/files/boost/1.63.0/boost_1_63_0.tar.gz
        UPDATE_COMMAND ""
        BUILD_IN_SOURCE 1
        CONFIGURE_COMMAND ./bootstrap.sh --prefix=${BOOST_INSTALL_DIR} --with-libraries=${BOOST_LIBRARIES_LIST} --without-icu --without-icu
        DEPENDS ${ZLIB_PROJECT}
        BUILD_COMMAND ./b2 install -j8 --disable-icu --ignore-site-config "cxxflags=-std=${STD_CXX} -fPIC" link=static threading=multi runtime-link=static
        INSTALL_COMMAND ""
    )

foreach (lib ${BOOST_LIBRARIES})
    list (APPEND LIBRARIES boost_${lib})
endforeach()
#---------------------------------------------------------

#------------------------ JsonCpp ------------------------
set (JSONCPP_INSTALL_DIR ${THITD_PARTY_OUTPUT_PATH}/jsoncpp)
set (JSONCPP_INCLUDE_DIR ${JSONCPP_INSTALL_DIR}/include)
set (JSONCPP_LIBRARIES_DIR ${JSONCPP_INSTALL_DIR}/lib)
include_directories (SYSTEM ${JSONCPP_INCLUDE_DIR})
link_directories(${JSONCPP_LIBRARIES_DIR})

set (JSONCPP_PROJECT "jsoncpp-project")
list (APPEND THIRD_PARTY_PROJECTS ${JSONCPP_PROJECT})

ExternalProject_Add (${JSONCPP_PROJECT}
            GIT_REPOSITORY https://github.com/open-source-parsers/jsoncpp.git
            GIT_TAG 1.8.0
            UPDATE_COMMAND ""
            BUILD_IN_SOURCE 1
            CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:INTERNAL=${JSONCPP_INSTALL_DIR} 
                -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS} 
                -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS} 
                -DJSONCPP_WITH_POST_BUILD_UNITTEST=OFF
                -DJSONCPP_WITH_PKGCONFIG_SUPPORT=OFF
                -DJSONCPP_WITH_CMAKE_PACKAGE=OFF
                -DBUILD_SHARED_LIBS=OFF
                -DBUILD_STATIC_LIBS=ON
                -DJSONCPP_WITH_TESTS=OFF
        )

list (APPEND LIBRARIES "jsoncpp")
#---------------------------------------------------------

#------------------------ Event --------------------------
set (EVENT_INSTALL_DIR ${THITD_PARTY_OUTPUT_PATH}/event)
set (EVENT_INCLUDE_DIR ${EVENT_INSTALL_DIR}/include)
set (EVENT_LIBRARIES_DIR ${EVENT_INSTALL_DIR}/lib)
include_directories (SYSTEM ${EVENT_INCLUDE_DIR})
link_directories(${EVENT_LIBRARIES_DIR})

set (EVENT_PROJECT "event-project")
list (APPEND THIRD_PARTY_PROJECTS ${EVENT_PROJECT})

ExternalProject_Add (${EVENT_PROJECT}
            GIT_REPOSITORY https://github.com/libevent/libevent.git
            GIT_TAG release-2.1.8-stable
            UPDATE_COMMAND ""
            BUILD_IN_SOURCE 1
            CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:INTERNAL=${EVENT_INSTALL_DIR} 
                -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS} 
                -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS} 
                -DEVENT__BUILD_SHARED_LIBRARIES=OFF
                -DEVENT__DISABLE_TESTS=ON
                -DEVENT__DISABLE_REGRESS=ON
                -DEVENT__DISABLE_SAMPLES=ON
                -DEVENT__DISABLE_BENCHMARK=ON
        )

list (APPEND LIBRARIES "event")
list (APPEND LIBRARIES "event_core")
list (APPEND LIBRARIES "event_extra")
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
            GIT_REPOSITORY https://github.com/postgres/postgres.git
            GIT_TAG REL9_6_2
            UPDATE_COMMAND ""
            BUILD_IN_SOURCE 1
            CONFIGURE_COMMAND ./configure --enable-thread-safety --without-readline --prefix=${LIBPQ_INSTALL_DIR}
            BUILD_IN_SOURCE 1
            BUILD_COMMAND make install -C src/interfaces/libpq
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory src/include ${LIBPQ_INSTALL_DIR}/include
            LOG_CONFIGURE 1
            LOG_BUILD 1
            LOG_INSTALL 1
        )

list (APPEND LIBRARIES "pq.a")
#---------------------------------------------------------

#------------------------ SQLITE -------------------------
set (SQLITE_INSTALL_DIR ${THITD_PARTY_OUTPUT_PATH}/sqlite)
set (SQLITE_INCLUDE_DIR ${SQLITE_INSTALL_DIR}/include)
set (SQLITE_LIBRARIES_DIR ${SQLITE_INSTALL_DIR}/lib)
include_directories (SYSTEM ${SQLITE_INCLUDE_DIR})
link_directories(${SQLITE_LIBRARIES_DIR})

set (SQLITE_PROJECT "sqlite-project")
list (APPEND THIRD_PARTY_PROJECTS ${SQLITE_PROJECT})

ExternalProject_Add (${SQLITE_PROJECT}
            URL https://www.sqlite.org/src/tarball/sqlite.tar.gz
            UPDATE_COMMAND ""
            BUILD_IN_SOURCE 1
            CONFIGURE_COMMAND ./configure --prefix=${SQLITE_INSTALL_DIR} --disable-readline --enable-shared=no --disable-amalgamation --enable-releasemode --disable-tcl --disable-load-extension CPPFLAGS=-fPIC CFLAGS=-fPIC
            BUILD_COMMAND make
            INSTALL_COMMAND make install
            LOG_CONFIGURE 1
            LOG_BUILD 1
            LOG_INSTALL 1
        )

list (APPEND LIBRARIES "sqlite3")
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
            GIT_REPOSITORY https://github.com/tdv/mif.git
            GIT_TAG master
            UPDATE_COMMAND ""
            BUILD_IN_SOURCE 1
            CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:INTERNAL=${MIF_INSTALL_DIR} 
                -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS} 
                -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS} 
                -DMIF_NEED_THIRD_PARTY_BUILD=OFF
                -DZLIB_INCLUDE_DIR=${ZLIB_INCLUDE_DIR}
                -DZLIB_LIBRARIES_DIR=${ZLIB_LIBRARIES_DIR}
                -DBOOST_INCLUDE_DIR=${BOOST_INCLUDE_DIR}
                -DBOOST_LIBRARIES_DIR=${BOOST_LIBRARIES_DIR}
                -DJSONCPP_INCLUDE_DIR=${JSONCPP_INCLUDE_DIR}
                -DJSONCPP_LIBRARIES_DIR=${JSONCPP_LIBRARIES_DIR}
                -DEVENT_INCLUDE_DIR=${EVENT_INCLUDE_DIR}
                -DEVENT_LIBRARIES_DIR=${EVENT_LIBRARIES_DIR}
                -DLIBPQ_INCLUDE_DIR=${LIBPQ_INCLUDE_DIR}
                -DLIBPQ_LIBRARIES_DIR=${LIBPQ_LIBRARIES_DIR}
                -DSQLITE_INCLUDE_DIR=${SQLITE_INCLUDE_DIR}
                -DSQLITE_LIBRARIES_DIR=${SQLITE_LIBRARIES_DIR}
            DEPENDS ${BOOST_PROJECT} ${JSONCPP_PROJECT} ${EVENT_PROJECT} ${LIBPQ_PROJECT} ${SQLITE_PROJECT}
            BUILD_COMMAND ${MAKE}
            INSTALL_COMMAND make install
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

add_executable(${PROJECT_LC} ${HEADERS} ${SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)
target_link_libraries(${PROJECT_LC} ${LIBRARIES})
add_dependencies (${PROJECT_LC} ${THIRD_PARTY_PROJECTS})
