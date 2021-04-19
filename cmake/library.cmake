include (cmake/version.cmake)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/mif/common/version.h.in ${CMAKE_CURRENT_SOURCE_DIR}/include/mif/common/version.h)

set (MIF_COMPILER_DEFINES )

if (MIF_WITH_POSTGRESQL)
    set (MIF_COMPILER_DEFINES "${MIF_COMPILER_DEFINES}\n#define MIF_WITH_POSTGRESQL")
endif()

if (MIF_PRETTY_JSON_WRITER)
    set (MIF_COMPILER_DEFINES "${MIF_COMPILER_DEFINES}\n#define MIF_PRETTY_JSON_WRITER")
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/mif/common/config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/include/mif/common/config.h)

set(MIF_SOURCES
    ${MIF_SOURCES}
    
    # Common
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/common/log.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/common/thread_pool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/common/uuid_generator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/common/hash_calc.cpp

    # Service
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/service/locator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/service/external.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/service/pool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/service/per_thread_pool.cpp

    # Remote
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/remote/factory.cpp

    # Net
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/net/client.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/net/clients/frame_reader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/net/clients/frame_writer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/net/clients/gzip_decompressor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/net/clients/gzip_compressor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/net/clients/parallel_handler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/net/tcp/server.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/net/tcp/clients.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/net/tcp/connection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/net/tcp/detail/session.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/net/http/server.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/net/http/detail/session.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/net/http/detail/utility.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/net/http/connection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/net/http/servlet.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/net/http/web_service.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/net/http/clients.cpp

    # Application
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/application/application.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/application/net_base_application.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/application/tcp_service.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/application/tcp_service_client.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/application/http_server.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/application/config/json.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/application/config/xml.cpp

    # DB
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/db/transaction.cpp
)

if (MIF_WITH_POSTGRESQL)
    set(MIF_SOURCES
        ${MIF_SOURCES}
            # DB
            ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/db/postgresql/connection.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/db/postgresql/detail/statement.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/db/postgresql/detail/recordset.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/mif/db/postgresql/connection_pool.cpp
        )
endif()

set (MIF_DEPENDENCIES_LIBRARIES
    ${MIF_DEPENDENCIES_LIBRARIES}
    ${BOOST_LIBRARIES}
    ${ZLIB_LIBRARIES}
)

if (MIF_WITH_POSTGRESQL)
    set (MIF_DEPENDENCIES_LIBRARIES
        ${LIBPQ_LIBRARIES}
    )
endif()

include_directories (include)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/src)

add_library ("${PROJECT_LC}_obj" OBJECT ${MIF_SOURCES})
if (MIF_NEED_THIRD_PARTY_BUILD)
    add_dependencies ("${PROJECT_LC}_obj" ${MIF_THIRD_PARTY_PROJECTS})
endif()

if (MIF_STATIC_LIBS)
    add_library (${PROJECT_LC} STATIC $<TARGET_OBJECTS:${PROJECT_LC}_obj>)
endif()

if (MIF_SHARED_LIBS)
    add_library (${PROJECT_LC}${MIF_SHARED_LIB_SUFFIX} SHARED $<TARGET_OBJECTS:${PROJECT_LC}_obj>)
    target_link_libraries (${PROJECT_LC}${MIF_SHARED_LIB_SUFFIX} ${MIF_DEPENDENCIES_LIBRARIES})
endif()
