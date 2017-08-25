cmake_minimum_required(VERSION 2.8)

include(ExternalProject)

project(my_app)
set(PROJECT ${PROJECT_NAME})
string(TOLOWER "${PROJECT}" PROJECT_LC)

set (STD_CXX "c++11")

set (THITD_PARTY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/third_party)
set (THIRD_PARTY_PROJECTS "")
set (LIBRARIES "")

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
        GIT_REPOSITORY https://github.com/boostorg/boost.git
        GIT_TAG boost-1.63.0
        BUILD_IN_SOURCE 1
        UPDATE_COMMAND ""
        CONFIGURE_COMMAND ./bootstrap.sh --prefix=${BOOST_INSTALL_DIR} --with-libraries=${BOOST_LIBRARIES_LIST} --without-icu --without-icu
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
            CONFIGURE_COMMAND ./configure --enable-thread-safety --without-readline --prefix=${LIBPQ_INSTALL_DIR}
            BUILD_COMMAND make -C src/interfaces/libpq
            INSTALL_COMMAND make -C src/interfaces/libpq install
                    && cp src/include/postgres_ext.h ${LIBPQ_INSTALL_DIR}/include
                    && cp src/include/pg_config_ext.h ${LIBPQ_INSTALL_DIR}/include
        )

list (APPEND LIBRARIES "pq.a")
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
)

add_executable(${PROJECT_LC} ${HEADERS} ${SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)
target_link_libraries(${PROJECT_LC} ${LIBRARIES})
add_dependencies (${PROJECT_LC} ${THIRD_PARTY_PROJECTS})
