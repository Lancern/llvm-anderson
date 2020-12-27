cmake_minimum_required(VERSION 3.10)
project(p2a)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in ${LLVM_DIR}")

set(CMAKE_CXX_STANDARD 14)

set(P2A_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

include_directories(${LLVM_INCLUDE_DIRS} "${P2A_INCLUDE_DIR}")
add_definitions(${LLVM_DEFINITIONS})

add_subdirectory(src)
