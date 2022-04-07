cmake_minimum_required(VERSION 3.10)

# set the project name
project(Sample)

set(NTNX_TOOLCHAIN_BUILDS_VERSION "82ee087e0bd6693a6f75c04db3ed205df8d84310")

list(APPEND CMAKE_PREFIX_PATH "/mnt/toolchain-builds/${NTNX_TOOLCHAIN_BUILDS_VERSION}.x86_64/lib/")
list(APPEND CMAKE_PREFIX_PATH "/mnt/toolchain-builds/${NTNX_TOOLCHAIN_BUILDS_VERSION}.x86_64/lib/cmake")

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# include header directories
include_directories(/mnt/toolchain-builds/${NTNX_TOOLCHAIN_BUILDS_VERSION}.x86_64/include)

add_library(sample_lib
            nutanix_span.cc
            nutanix_observability.cc)

find_library(THRIFT_LIBRARIES thrift)
find_library(OPENTELEMETRY_CPP_COMMON_LIBRARIES opentelemetry_common)
find_library(OPENTELEMETRY_CPP_JAEGER_EXPORTED_LIBRARIES opentelemetry_exporter_jaeger_trace)
find_library(OPENTELEMETRY_CPP_OSTREAM_LIBRARIES opentelemetry_exporter_ostream_span)
find_library(OPENTELEMETRY_CPP_METRICS_LIBRARIES opentelemetry_metrics)
find_library(OPENTELEMETRY_CPP_RESOURCES_LIBRARIES opentelemetry_resources)
find_library(OPENTELEMETRY_CPP_TRACE_LIBRARIES opentelemetry_trace)
find_library(OPENTELEMETRY_CPP_VERSION_LIBRARIES opentelemetry_version)
find_library(HTTP_CLIENT_CURL_LIBRARIES http_client_curl)
find_package(absl)
Set(ABSL_LIBRARIES absl::bad_variant_access;absl::any;absl::base;absl::bits;absl::city)

target_link_libraries(sample_lib 
    ${OPENTELEMETRY_CPP_COMMON_LIBRARIES}
    ${OPENTELEMETRY_CPP_JAEGER_EXPORTED_LIBRARIES}
    ${OPENTELEMETRY_CPP_OSTREAM_LIBRARIES}
    ${OPENTELEMETRY_CPP_METRICS_LIBRARIES}
    ${OPENTELEMETRY_CPP_RESOURCES_LIBRARIES}
    ${OPENTELEMETRY_CPP_TRACE_LIBRARIES}
    ${OPENTELEMETRY_CPP_VERSION_LIBRARIES}
    ${HTTP_CLIENT_CURL_LIBRARIES}
    ${THRIFT_LIBRARIES}
    ${ABSL_LIBRARIES}
)

# add the executable
add_executable(Sample sample.cxx)

target_link_libraries(Sample sample_lib)