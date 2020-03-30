cmake_minimum_required(VERSION 3.12)
project(ironBeetroot)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin_new)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)
include_directories(uv)

file(GLOB COMMON_SOURCES common/*.c common/*.h common/net/*.c common/net/*.h)

file(GLOB ARM_SOURCES arm/*.c arm/*.h)
add_executable(arm ${ARM_SOURCES} ${COMMON_SOURCES})
target_link_libraries(arm uv_a)

file(GLOB CUSTOMER consumer/*.c consumer/*.h)
add_executable(customer ${CUSTOMER} ${COMMON_SOURCES})
target_link_libraries(customer uv_a)

file(GLOB LEADER_SOURCES leader/*.c leader/*.h)
add_executable(leader ${LEADER_SOURCES} ${COMMON_SOURCES})
target_link_libraries(leader uv_a)

file(GLOB HALL_SURCES hall/*.c hall/*.h)
add_executable(hall ${HALL_SOURCES} ${COMMON_SOURCES})
target_link_libraries(hall uv_a)

#TODO: show.exe
