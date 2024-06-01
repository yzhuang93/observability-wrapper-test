cmake_minimum_required(VERSION 3.10)

# set the project name
project(Sample)

set(NTNX_TOOLCHAIN_BUILDS_VERSION "85ebebaf3dd05cac478fa39c056392a66232f9c7")

list(APPEND CMAKE_PREFIX_PATH "/mnt/toolchain-builds/${NTNX_TOOLCHAIN_BUILDS_VERSION}.x86_64/lib/")
list(APPEND CMAKE_PREFIX_PATH "/mnt/toolchain-builds/${NTNX_TOOLCHAIN_BUILDS_VERSION}.x86_64/lib/cmake")

SET(PLATFORM_PREFIX, /opt/cross/el8.5-x86_64/)
SET(ARCH, x86_64)
SET(CLANG_VERSION 13.0.1)
SET(CMAKE_CUSTOM_SYSROOT /opt/cross/el8.5-x86_64/sysroot)
SET(CMAKE_CUSTOM_CLANG_PLATFORM_PREFIX "/opt/cross/el8.5-x86_64/clang-${CLANG_VERSION}")

# Define CMAKE_CXX_FLAGS: https://sourcegraph.ntnxdpro.com/devtools-master@a38a3511a40ac367b1f343b633c7f55c1428077a/-/blob/makefiles/CMakeLists.txt.common?L117
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-enum-compare")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-ignored-qualifiers")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GNU_SOURCE -D__STDC_FORMAT_MACROS")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CUSTOM_CXX_SYMBOLS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CUSTOM_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=nehalem -mtune=haswell")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS}")

# Define CLANG related flags: https://sourcegraph.ntnxdpro.com/devtools-master@a38a3511a40ac367b1f343b633c7f55c1428077a/-/blob/makefiles/CMakeLists.txt.common?L258
set(CLANG_CPP_FLAGS "${CLANG_CPP_FLAGS} -B ${CMAKE_CUSTOM_CLANG_PLATFORM_PREFIX}/bin")
set(CLANG_CPP_FLAGS "${CLANG_CPP_FLAGS} -target x86_64-redhat-linux")
set(CLANG_CPP_FLAGS "${CLANG_CPP_FLAGS} --sysroot ${CMAKE_CUSTOM_SYSROOT}")
set(CLANG_CPP_FLAGS "${CLANG_CPP_FLAGS} -resource-dir=${CMAKE_CUSTOM_CLANG_PLATFORM_PREFIX}/lib/clang")
set(CLANG_CPP_FLAGS "${CLANG_CPP_FLAGS} -rtlib=compiler-rt")
set(CLANG_CPP_FLAGS "${CLANG_CPP_FLAGS} -unwindlib=libunwind")
set(CLANG_CPP_FLAGS "${CLANG_CPP_FLAGS} -stdlib=libc++")
set(CLANG_CPP_FLAGS "${CLANG_CPP_FLAGS} -stdlib++-isystem${CMAKE_CUSTOM_CLANG_PLATFORM_PREFIX}/include/c++/v1")
set(CLANG_CPP_FLAGS "${CLANG_CPP_FLAGS} -L${CMAKE_CUSTOM_CLANG_PLATFORM_PREFIX}/lib")
set(CLANG_CPP_FLAGS "${CLANG_CPP_FLAGS} -fuse-ld=lld")
set(CLANG_CPP_FLAGS "${CLANG_CPP_FLAGS} -Qunused-arguments")
set(CLANG_CPP_FLAGS "${CLANG_CPP_FLAGS} -ftemplate-depth=512")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CLANG_CPP_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CLANG_CPP_FLAGS}")

# set(CMAKE_CUSTOM_LINKFLAGS "${CMAKE_CUSTOM_LINKFLAGS} -lstdc++")
set(CMAKE_CUSTOM_LINKFLAGS "${CMAKE_CUSTOM_LINKFLAGS} -Wl")
set(CMAKE_CUSTOM_LINKFLAGS "${CMAKE_CUSTOM_LINKFLAGS} -dynamic-linker=${CMAKE_CUSTOM_SYSROOT}/lib64/ld-linux-x86-64.so.2")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${CMAKE_CUSTOM_LINKFLAGS}")

# include header directories
include_directories(/mnt/toolchain-builds/${NTNX_TOOLCHAIN_BUILDS_VERSION}.x86_64/include)

find_library(JSON_CPP jsoncpp REQUIRED)

# add the executable
add_executable(Sample sample.cxx)

target_link_libraries(Sample ${JSON_CPP})