cmake_minimum_required(VERSION 3.10)
project(SpineSkeletonDataConverter)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add spine-c/spine-cpp for syntax prompting. These won't be linked to the final executable.
# Comment out if you don't need syntax prompting.

macro(add_spine_c version)
    file(GLOB SPINE_C
        "spine/spine-c-${version}/include/spine/*.h"
        "spine/spine-c-${version}/src/spine/*.c")
    add_library(spine_c_${version} ${SPINE_C})
    target_include_directories(spine_c_${version} PRIVATE "spine/spine-c-${version}/include")
endmacro()

macro(add_spine_cpp version)
    file(GLOB SPINE_CPP
        "spine/spine-cpp-${version}/include/spine/*.h"
        "spine/spine-cpp-${version}/src/spine/*.cpp")
    add_library(spine_cpp_${version} ${SPINE_CPP})
    target_include_directories(spine_cpp_${version} PRIVATE "spine/spine-cpp-${version}/include")
endmacro()

add_spine_c(21)
add_spine_c(35)
add_spine_c(36)
add_spine_cpp(37)
add_spine_cpp(38)
add_spine_cpp(40)
add_spine_cpp(41)
add_spine_cpp(42)

add_executable(SpineSkeletonDataConverter
    "include/json.hpp"
    "src/main.cpp"
    "src/common.cpp"
    "src/CurveConverter.cpp"
    "src/SkeletonData36BinaryReader.cpp"
    "src/SkeletonData36BinaryWriter.cpp"
    "src/SkeletonData36JsonReader.cpp"
    "src/SkeletonData36JsonWriter.cpp"
    "src/SkeletonData37BinaryReader.cpp"
    "src/SkeletonData37BinaryWriter.cpp"
    "src/SkeletonData37JsonReader.cpp"
    "src/SkeletonData37JsonWriter.cpp"
    "src/SkeletonData38BinaryReader.cpp"
    "src/SkeletonData38BinaryWriter.cpp"
    "src/SkeletonData38JsonReader.cpp"
    "src/SkeletonData38JsonWriter.cpp"
    "src/SkeletonData40BinaryReader.cpp"
    "src/SkeletonData40BinaryWriter.cpp"
    "src/SkeletonData40JsonReader.cpp"
    "src/SkeletonData40JsonWriter.cpp"
    "src/SkeletonData41BinaryReader.cpp"
    "src/SkeletonData41BinaryWriter.cpp"
    "src/SkeletonData41JsonReader.cpp"
    "src/SkeletonData41JsonWriter.cpp"
    "src/SkeletonData42BinaryReader.cpp"
    "src/SkeletonData42BinaryWriter.cpp"
    "src/SkeletonData42JsonReader.cpp"
    "src/SkeletonData42JsonWriter.cpp"
)

target_include_directories(SpineSkeletonDataConverter PRIVATE "include")
