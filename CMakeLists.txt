cmake_minimum_required(VERSION 3.14)
project(metakv)

# GoogleTest requires at least C++11
set(CMAKE_CXX_STANDARD 20)

add_subdirectory("third_party/googletest")
add_subdirectory("proto")

set(JEMMLLOC_ENABLE ON)

if (JEMMLLOC_ENABLE)
    add_definitions("-L`jemalloc-config --libdir` -Wl,-rpath,`jemalloc-config --libdir` -ljemalloc `jemalloc-config --libs`")
endif()
add_definitions("-Wall -O0 -g")
include_directories(${PROTO_GEN_CXX_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/file)
include_directories(${CMAKE_SOURCE_DIR}/util)
include_directories(${CMAKE_SOURCE_DIR}/lsm)
include_directories(${CMAKE_SOURCE_DIR}/memtable)
include_directories(${CMAKE_SOURCE_DIR}/compaction)
include_directories(${CMAKE_SOURCE_DIR}/table)
include_directories(${CMAKE_SOURCE_DIR}/recovery)
include_directories(${CMAKE_SOURCE_DIR}/third_party/parallel-hashmap/parallel_hashmap)



set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(install_gtest OFF)
set(install_gmock OFF)
set(build_gmock ON)
include_directories(
    third_party/xxhashct
)

set(SRC /home/parallels/metakv/)

FILE (GLOB ALL_SOURCES "*.cpp" 
    "./file/*.cpp" 
    "./util/*.cpp" 
    "./util/*.cc" 
    "./lsm/*.cpp"
    "./table/*.cpp"
    "./compaction/*.cpp"
    "./memtable/*.cpp"
    "./recovery/*.cpp" )

message("hheloo" ${ALL_SOURCES})

function(meta_test test_file)
    get_filename_component(test_target_name "${test_file}" NAME_WE)
    add_executable("${test_target_name}" "")
    target_sources("${test_target_name}" PRIVATE  "${test_file}" ${ALL_SOURCES})
    target_link_libraries("${test_target_name}" gmock gtest proto_gen_cxx)
    add_test(NAME "${test_target_name}" COMMAND "${test_target_name}")
endfunction(meta_test)

meta_test("test/skipListsTest.cpp")
meta_test("test/lsm_test.cpp")
meta_test("test/bloomFilterTest.cpp")
meta_test("test/memtable_test.cpp")
meta_test("test/recovery_test.cpp")
meta_test("test/thread_test.cpp")

execute_process (
    COMMAND bash -c "rm -rf work_test*"
    OUTPUT_VARIABLE outVar
)
