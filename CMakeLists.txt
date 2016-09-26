cmake_minimum_required(VERSION 3.6)
project(Hypersonic)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

file(GLOB SOURCE_FILES "src/*.cpp")
add_executable(Hypersonic ${SOURCE_FILES})