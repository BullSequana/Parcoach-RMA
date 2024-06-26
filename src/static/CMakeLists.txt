#
# Copyright (c) 2020-2024 Bull S.A.S. All rights reserved.
# Copyright (c) 2020-2024 Inria All rights reserved.
#

# This file is part of Parcoach RMA
# Parcoach RMA is free software; you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation; either version 2.1 of the License, or
# (at your option) any later version.
# Parcoach RMA is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along with Parcoach RMA.
# If not, see <https://www.gnu.org/licenses/>.


cmake_minimum_required(VERSION 3.16)
project(parcoachRMA)

message("You need LLVM 9 or higher to build this project")
message("This is PARCOACH RMA")

option(COVERAGE "Build with coverage" OFF)

find_package(LLVM REQUIRED CONFIG)

if((LLVM_PACKAGE_VERSION VERSION_LESS 9))
  message(FATAL_ERROR "Uncompatible LLVM version found ${LLVM_PACKAGE_VERSION}")
endif()

if((LLVM_PACKAGE_VERSION VERSION_GREATER 9))
  # c++14
  set(CMAKE_CXX_STANDARD 14)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
else()
  # c++11
  set(CMAKE_CXX_STANDARD 11)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION} in ${LLVM_INSTALL_PREFIX}")
message(STATUS "Using LLVM binaries in: ${LLVM_TOOLS_BINARY_DIR}")
# FIXME: Not sure binary compiled with GCC would be always compatible with
# LLVM's opt. Maybe we should disable CMAKE_CXX_COMPILER and force using the
# clang provided with the LLVM release.
message(STATUS "Using CXX Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "Using C Compiler: ${CMAKE_C_COMPILER}")

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

find_package(MPI)

# Make the LLVM definitions globally available.
add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})

# Activate all warnings
add_compile_options(-Wall -Wno-int-to-pointer-cast)
if (COVERAGE)
    add_compile_options(-g --coverage)
    add_link_options(-lgcov --coverage)
endif(COVERAGE)

add_subdirectory(src)
add_subdirectory(lib)

enable_testing()
add_subdirectory(tests)
