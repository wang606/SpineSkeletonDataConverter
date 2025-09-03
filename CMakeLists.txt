cmake_minimum_required(VERSION 3.10)
project(SpineSkeletonDataConverter)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

macro(add_spine_cpp version)
    file(GLOB SPINE_CPP
        "spine/spine-cpp-${version}/include/spine/*.h"
        "spine/spine-cpp-${version}/src/spine/*.cpp")
    add_library(spine_cpp_${version} ${SPINE_CPP})
    target_include_directories(spine_cpp_${version} PRIVATE "spine/spine-cpp-${version}/include")
endmacro()

add_spine_cpp(37)
add_spine_cpp(38)
add_spine_cpp(40)
add_spine_cpp(41)
add_spine_cpp(42)

add_executable(SpineSkeletonDataConverter
    "include/json.hpp"
    "src/main.cpp"
    "src/common.cpp"
    "src/SkeletonData42BinaryReader.cpp"
    "src/SkeletonData42BinaryWriter.cpp"
    "src/SkeletonData42JsonReader.cpp"
    "src/SkeletonData42JsonWriter.cpp"
)

target_include_directories(SpineSkeletonDataConverter PRIVATE "include")
