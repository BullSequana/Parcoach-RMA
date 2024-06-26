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


set(EXPECTED_SUCCESS_TESTS
  )

set(EXPECTED_FAILURE_TESTS
  CodeFenceCreateGetLoad.c
  )

set(CLANG_BIN ${LLVM_TOOLS_BINARY_DIR}/clang)
set(BC_FLAGS -g -c -emit-llvm -I${MPI_C_HEADER_DIR})
set(ALL_TESTS_TARGETS)

configure_file(test_mpi.in test_mpi @ONLY)

foreach(SRC IN ITEMS ${EXPECTED_SUCCESS_TESTS} ${EXPECTED_FAILURE_TESTS})
  get_filename_component(TEST_NAME ${SRC} NAME_WE)
  set(TEST_BC ${TEST_NAME}.bc)
  set(TEST_TARGET tests_MPI_${TEST_NAME})
  # Append the target to all tests targets
  set(ALL_TESTS_TARGETS ${ALL_TESTS_TARGETS} ${TEST_TARGET})
  message("Adding test ${TEST_NAME}")
  add_custom_command(
    OUTPUT ${TEST_BC}
    COMMAND ${CLANG_BIN} ${BC_FLAGS} -o ${TEST_BC} ${CMAKE_CURRENT_SOURCE_DIR}/${SRC}
    DEPENDS parcoachRMA ${SRC}
    )
  add_custom_target(${TEST_TARGET} DEPENDS ${TEST_BC})
  add_test(NAME ${TEST_TARGET} COMMAND test_mpi ${CMAKE_CURRENT_SOURCE_DIR}/${SRC} ${CMAKE_CURRENT_SOURCE_DIR}/${SRC}.expected)
endforeach()

# Explicit mark expected failures
foreach(SRC IN ITEMS ${EXPECTED_FAILURE_TESTS})
  get_filename_component(TEST_NAME ${SRC} NAME_WE)
  set_tests_properties(tests_MPI_${TEST_NAME} PROPERTIES WILL_FAIL TRUE)
endforeach()

# Add a phony depend so that precompiled .bc are always built when running building aSSA
add_custom_target(tests_MPI ALL DEPENDS ${ALL_TESTS_TARGETS})
