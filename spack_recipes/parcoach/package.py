#
# Copyright (c) 2020-2024 Bull S.A.S. All rights reserved.
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


from spack.package import *
import os


class Parcoach(CMakePackage):
    """Analysis tool for MPI programs"""

    homepage = "https://parcoach.github.io/index.html"
    git      = "https://github.com/parcoach/parcoach.git"
    url      = "https://github.com/parcoach/parcoach/archive/refs/tags/2.4.0.tar.gz"

    version("2.4.0", sha256="7107be7fd2d814b5118d2213f5ceda48d24d3caf97c8feb19e7991e0ebaf2455")

    depends_on('classicflang@15.0.3%gcc')
    depends_on('cmake@3.24%gcc', type='build')
    depends_on('openmpi%clang')

    def cmake_args(self):
        cache_path = os.path.join(self.stage.source_path, "caches", "Release-shared.cmake")
        args = [
                # Preload cmake cache for the shared release
                f"-C {cache_path}",
                self.define("CMAKE_CXX_FLAGS", "-L/usr/local/software/skylake/Stages/2023/software/GCCcore/11.3.0/lib64 -lstdc++"),
                self.define("CMAKE_C_COMPILER", "clang"),
                self.define("CMAKE_CXX_COMPILER", "clang++"),
                self.define("CMAKE_Fortran_COMPILER", "flang"),
                self.define("CMAKE_Fortran_COMPILER_ID", "LLVMFlang"),
                ]
        return args

    #  def setup_build_environment(self, env):
    #      env.set('CC', 'clang')
    #      env.set('CXX', 'clang++')
    #      env.set('FC', 'flang')

    def setup_run_environment(self, env):
        # Ideally we would ask user to use CMake integration, and not "pollute"
        # these common flags. This is only useful for instrumentation.
        env.prepend_path("LD_LIBRARY_PATH", self.prefix.lib)
        env.prepend_path("LIBRARY_PATH", self.prefix.lib)
