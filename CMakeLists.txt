cmake_minimum_required(VERSION 3.14)
project(metakv)

# GoogleTest requires at least C++11
set(CMAKE_CXX_STANDARD 17)

add_subdirectory("third_party/googletest")
add_subdirectory("third_party/json")
add_definitions("-Wall -O0 -g")

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(install_gtest OFF)
set(install_gmock OFF)
set(build_gmock ON)
include_directories(
    third_party/xxhashct
)

function(meta_test test_file)
    get_filename_component(test_target_name "${test_file}" NAME_WE)
    add_executable("${test_target_name}" "")
    target_sources("${test_target_name}" PRIVATE  "${test_file}" crc32c.cc)
    target_link_libraries("${test_target_name}" gmock gtest nlohmann_json::nlohmann_json)
    add_test(NAME "${test_target_name}" COMMAND "${test_target_name}")
endfunction(meta_test)

meta_test("skipListsTest.cpp")
meta_test("lsm_test.cpp")
meta_test("bloomFilterTest.cpp")
