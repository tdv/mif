cmake_minimum_required(VERSION 2.8)

project(http_crud)
set(PROJECT ${PROJECT_NAME})
string(TOLOWER "${PROJECT}" PROJECT_LC)

include (../common/cmake/mif.cmake)

include_directories(${CMAKE_SOURCE_DIR}/../common/include)

set(COMMON_HEADERS
    ${COMMON_HEADERS}
)

set(HEADERS
    ${HEADERS}
    ${COMMON_HEADERS}
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

set(SOURCES
    ${SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/employee_service.cpp
)

add_executable(${PROJECT_LC} ${HEADERS} ${SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)
target_link_libraries(${PROJECT_LC} ${LIBRARIES})
