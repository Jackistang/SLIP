cmake_minimum_required(VERSION 3.12)

project(tests)

set(CMAKE_BUILD_TYPE "Debug")

# For Cmake Modules
set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/../3rd-party)
list(APPEND CMAKE_MODULE_PATH "${THIRD_PARTY_DIR}/cmake-modules")

# Look for installed version of CppUTest
if(DEFINED ENV{CPPUTEST_HOME})
    message(STATUS "Using CppUTest home: $ENV{CPPUTEST_HOME}")
    set(CPPUTEST_INCLUDE_DIRS $ENV{CPPUTEST_HOME}/include)
    set(CPPUTEST_LIBRARIES $ENV{CPPUTEST_HOME}/lib)
    set(CPPUTEST_LDFLAGS CppUTest CppUTestExt)
else()
    find_package(PkgConfig REQUIRED)
    pkg_search_module(CPPUTEST REQUIRED cpputest>=3.8)
    message(STATUS "Found CppUTest version ${CPPUTEST_VERSION}")
endif()

# Test files
add_executable(test_slip
    test.cpp
    ../src/slip.c)

target_include_directories(test_slip PRIVATE
    ${CPPUTEST_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../src)

target_link_directories(test_slip PRIVATE
    ${CPPUTEST_LIBRARIES})

target_link_libraries(test_slip PRIVATE 
    ${CPPUTEST_LDFLAGS} 
    gcov)

# Using ctest
enable_testing()
add_test(NAME test_slip COMMAND test_slip)

# For coverage
if(WITH_COVERAGE)
    include(CodeCoverage)
    append_coverage_compiler_flags()

    setup_target_for_coverage_lcov(
        NAME coverage
        EXECUTABLE ctest
        EXCLUDE "${CMAKE_SOURCE_DIR}/*" "${THIRD_PARTY_DIR}/*")
endif(WITH_COVERAGE)