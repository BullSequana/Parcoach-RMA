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


add_llvm_library(parcoachRMA MODULE
	parcoachRMA.cpp

  PLUGIN_TOOL
  opt
  )

if(APPLE)
  # The magic line.
  # We're building a shared lib, tell OSX to resolve the symbol when actually
  # loading the library.
  # If we don't set this and manually add the appropriate LLVM libs, loading
  # the plugin will silently fail (!!!) because of duplicate symbols.
  set_target_properties(parcoachRMA PROPERTIES
    LINK_FLAGS "-undefined dynamic_lookup"
    )
endif()

set(PARCOACH_LIB ${CMAKE_CURRENT_BINARY_DIR}/parcoachRMA${CMAKE_SHARED_LIBRARY_SUFFIX})
configure_file(parcoach.in ${CMAKE_BINARY_DIR}/parcoach @ONLY)
configure_file(parcoachBC.in ${CMAKE_BINARY_DIR}/parcoachBC @ONLY)

if(COVERAGE)
    add_compile_options(-g --coverage)
    add_link_options(-lgcov --coverage)
endif(COVERAGE)
