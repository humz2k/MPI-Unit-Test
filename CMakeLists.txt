cmake_minimum_required(VERSION 3.24)

# for printing variables
include(CMakePrintHelpers)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(MPI-UNIT-TEST LANGUAGES CXX)

# We need MPI and PkgConfig
find_package(MPI REQUIRED)

# swfft-gpu library
add_library(mpitest INTERFACE)

# our include directory
target_include_directories(mpitest INTERFACE include)

# need MPI
target_link_libraries(mpitest INTERFACE MPI::MPI_CXX)

if (${PROJECT_IS_TOP_LEVEL})
add_executable(example example.cpp)
target_link_libraries(example PUBLIC mpitest)
endif()