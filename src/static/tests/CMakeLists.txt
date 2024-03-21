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


find_package(Python3 COMPONENTS Interpreter REQUIRED)

# For now only MPI tests are supported through CMake
find_package(MPI)
if(MPI_FOUND)
  message(STATUS "Configuring MPI tests")
  add_subdirectory(MPI-RMA)
else()
  message(STATUS "MPI was *NOT* found, disabling MPI tests")
endif()
