cmake_minimum_required(VERSION 3.10)
project(SpineSkeletonDataConverter)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

macro(add_test_skel2json_library version)
    file(GLOB SPINE_CPP
        "test/spine/spine-cpp-${version}/include/spine/*.h"
        "test/spine/spine-cpp-${version}/src/spine/*.cpp")
    add_library(test_skel2json${version} SHARED 
        ${SPINE_CPP}
        "test/test_skel2json${version}.cpp")
    target_include_directories(test_skel2json${version} PRIVATE "test/spine/spine-cpp-${version}/include")
endmacro()

add_test_skel2json_library(37)
add_test_skel2json_library(38)
add_test_skel2json_library(40)
add_test_skel2json_library(41)
add_test_skel2json_library(42)

add_executable(skel2json
    "src/json.hpp"
    "src/main.cpp"
    "src/skel2json37.cpp"
    "src/skel2json38.cpp"
    "src/skel2json40.cpp"
    "src/skel2json41.cpp"
    "src/skel2json42.cpp"
)

add_executable(test_skel2json
    "test/test.cpp"
)
target_link_libraries(test_skel2json PRIVATE
    test_skel2json37
    test_skel2json38
    test_skel2json40
    test_skel2json41
    test_skel2json42
)

# Install rules - 将所有生成的文件复制到target目录
install(TARGETS skel2json test_skel2json
    RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/target
)

install(TARGETS test_skel2json37 test_skel2json38 test_skel2json40 test_skel2json41 test_skel2json42
    LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/target
    RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/target
)
