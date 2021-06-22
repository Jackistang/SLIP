cmake_minimum_required(VERSION  3.15)

project(slip)

set (CMAKE_BUILD_TYPE "Debug")

set(SOURCES
    slip.c
    test_slip.c
)

add_executable(slip ${SOURCES})

target_include_directories(slip
    PRIVATE
    ${PROJECT_SOURCE_DIR})

target_link_libraries(slip
    PRIVATE
    cunit)
