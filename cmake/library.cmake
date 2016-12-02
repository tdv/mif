set(MIF_SOURCES
    ${MIF_SOURCES}
    
    #Common
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/common/log.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/common/thread_pool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/common/uuid_generator.cpp

    #Net
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/net/client.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/net/clients/frame_reader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/net/clients/frame_writer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/net/clients/gzip_decompressor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/net/clients/gzip_compressor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/net/tcp_server.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/net/tcp_clients.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/net/detail/tcp_session.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/net/http/server.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/net/http/detail/server_thread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/net/http/detail/lib_event_initializer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/net/http/detail/server.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/net/http/detail/input_pack.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/net/http/detail/output_pack.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/net/http/detail/utility.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/net/http/connection.cpp
)

set (MIF_DEPENDENCIES_LIBRARIES
    ${MIF_DEPENDENCIES_LIBRARIES}
    ${BOOST_LIBRARIES}
    ${JSONCPP_LIBRARIES}
    ${ZLIB_LIBRARIES}
#    ${EVENT_LIBRARIES}
)

include_directories (include)
include_directories (MIF_SOURCES)

add_library ("${PROJECT_LC}_obj" OBJECT ${MIF_SOURCES})
add_dependencies ("${PROJECT_LC}_obj" ${MIF_THIRD_PARTY_PROJECTS})

if (MIF_STATIC_LIBS)
    add_library (${PROJECT_LC} STATIC $<TARGET_OBJECTS:${PROJECT_LC}_obj>)
endif()

if (MIF_SHARED_LIBS)
    add_library (${PROJECT_LC}${MIF_SHARED_LIB_SUFFIX} SHARED $<TARGET_OBJECTS:${PROJECT_LC}_obj>)
    target_link_libraries (${PROJECT_LC}${MIF_SHARED_LIB_SUFFIX} ${MIF_DEPENDENCIES_LIBRARIES})
endif()
