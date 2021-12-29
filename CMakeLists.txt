cmake_minimum_required(VERSION 3.14)
project(metakv)

# GoogleTest requires at least C++11
set(CMAKE_CXX_STANDARD 17)

add_subdirectory("third_party/googletest")
add_subdirectory("third_party/json")
add_subdirectory("proto")

set(JEMMLLOC_ENABLE ON)

if (JEMMLLOC_ENABLE)
    add_definitions("-L`jemalloc-config --libdir` -Wl,-rpath,`jemalloc-config --libdir` -ljemalloc `jemalloc-config --libs`")
endif()
add_definitions("-Wall -O0 -g")
include_directories(${PROTO_GEN_CXX_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/file)
include_directories(${CMAKE_SOURCE_DIR}/third_party/parallel-hashmap/parallel_hashmap)



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
    target_sources("${test_target_name}" PRIVATE  "${test_file}" crc32c.cc codec.cpp file/mmapfile.cpp)
    target_link_libraries("${test_target_name}" gmock gtest nlohmann_json::nlohmann_json proto_gen_cxx)
    add_test(NAME "${test_target_name}" COMMAND "${test_target_name}")
endfunction(meta_test)

meta_test("skipListsTest.cpp")
meta_test("lsm_test.cpp")
meta_test("bloomFilterTest.cpp")
meta_test("memtable_test.cpp")
meta_test("recovery_test.cpp")

execute_process (
    COMMAND bash -c "rm -rf work_test*"
    OUTPUT_VARIABLE outVar
)
