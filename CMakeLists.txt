cmake_minimum_required(VERSION 3.10)

# set the project name
project(Sample)

# add the executable
add_executable(Sample sample.cxx)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# include opentracing
include_directories(/usr/local/include)
find_library(THRIFT_LIB thrift)
find_library(OPENTRACING_LIB opentracing)
find_library(YAML_CPP_LIB yaml-cpp)
find_library(JAEGERTRACING_LIB jaegertracing)
find_library(CURL curl)
target_link_libraries(Sample PRIVATE ${OPENTRACING_LIB}
                                     ${YAML_CPP_LIB}
                                     ${THRIFT_LIB}
                                     ${JAEGERTRACING_LIB}
                                     ${CURL})
