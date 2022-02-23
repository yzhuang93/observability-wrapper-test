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

find_package(CURL)
find_package(Thrift)
find_package(opentelemetry-cpp CONFIG REQUIRED HINTS "/usr/local/lib64/cmake/opentelemetry-cpp/")

target_include_directories(Sample PRIVATE ${OPENTELEMETRY_CPP_INCLUDE_DIRS})
target_link_libraries(Sample PRIVATE ${OPENTELEMETRY_CPP_LIBRARIES})