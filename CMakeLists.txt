cmake_minimum_required(VERSION  3.12)

project(slip)

set (CMAKE_BUILD_TYPE "Debug")

set(SOURCES
    slip.c
    test_slip.c
    3rd-party/ringbuffer.c
)

add_executable(slip ${SOURCES})

target_include_directories(slip
    PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/3rd-party)

target_link_libraries(slip
    PRIVATE
    cunit)
