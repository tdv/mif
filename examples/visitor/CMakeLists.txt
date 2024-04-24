cmake_minimum_required(VERSION 3.27)

project(Visitor)
set(PROJECT ${PROJECT_NAME})
string(TOLOWER "${PROJECT}" PROJECT_LC)

include (../common/cmake/mif.cmake)

include_directories(${CMAKE_SOURCE_DIR}/common/include)

set(COMMON_HEADERS
    ${COMMON_HEADERS}
    ${CMAKE_SOURCE_DIR}/common/include
)

set(SERVER_HEADERS
    ${SERVER_HEADERS}
    ${COMMON_HEADERS}
)

set(SERVER_SOURCES
    ${SERVER_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/server/src/service.cpp
)

set(CLIENT_HEADERS
    ${SERVER_HEADERS}
    ${COMMON_HEADERS}
)

set(CLIENT_SOURCES
    ${CLIENT_SOURCES}
)

add_executable(${PROJECT_SERVER_NAME} ${SERVER_HEADERS} ${SERVER_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/server/src/main.cpp)
target_link_libraries(${PROJECT_SERVER_NAME} ${LIBRARIES})

add_executable(${PROJECT_CLIENT_NAME} ${CLIENT_HEADERS} ${CLIENT_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/client/src/main.cpp)
target_link_libraries(${PROJECT_CLIENT_NAME} ${LIBRARIES})
