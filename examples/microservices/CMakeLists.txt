cmake_minimum_required(VERSION 3.27)

project(ComplexType)
set(PROJECT ${PROJECT_NAME})
string(TOLOWER "${PROJECT}" PROJECT_LC)

set(SERVICE_PROJECT "service")
set(STORAGE_PROJECT "storage")

include (../common/cmake/mif.cmake)

include_directories(${CMAKE_SOURCE_DIR}/common/include)

set(COMMON_HEADERS
    ${COMMON_HEADERS}
    ${CMAKE_SOURCE_DIR}/common/include
)

set(SERVICE_SOURCES
    ${SERVICE_SOURCES}
    ${COMMON_HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/service/src
)

set(SERVICE_SOURCES
    ${SERVICE_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/service/src/employee_service.cpp
)

set(STORAGE_HEADERS
    ${STORAGE_HEADERS}
    ${COMMON_HEADERS}
)

set(STORAGE_SOURCES
    ${STORAGE_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/storage/src/pg_employee_storage.cpp
)

add_executable(${SERVICE_PROJECT} ${SERVICE_HEADERS} ${SERVICE_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/service/src/main.cpp)
target_link_libraries(${SERVICE_PROJECT} ${LIBRARIES})

add_executable(${STORAGE_PROJECT} ${STORAGE_HEADERS} ${STORAGE_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/storage/src/main.cpp)
target_link_libraries(${STORAGE_PROJECT} ${LIBRARIES})
