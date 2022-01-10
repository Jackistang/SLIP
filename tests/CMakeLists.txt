cmake_minimum_required(VERSION 3.12)

project(tests)


# CppUTest
include(FetchContent)
FetchContent_Declare(
    CppUTest
    GIT_REPOSITORY https://github.com.cnpmjs.org/cpputest/cpputest.git
    GIT_TAG        latest-passing-build # or use release tag, eg. v3.8
)
# Set this to ON if you want to have the CppUTests in your project as well.
set(TESTS OFF CACHE BOOL "Switch off CppUTest Test build")
FetchContent_MakeAvailable(CppUTest)


add_executable(test_slip
    test.cpp
    ../src/slip.c)

target_include_directories(test_slip PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src)

target_link_libraries(test_slip PRIVATE CppUTest CppUTestExt)

# Using ctest
enable_testing()
add_test(NAME test_slip COMMAND test_slip)