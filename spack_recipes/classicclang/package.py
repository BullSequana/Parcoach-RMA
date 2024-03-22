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


class Classicclang(CMakePackage):
    """ClassicClang compiler with support for flang (F18)"""

    homepage = "https://github.com/flang-compiler"
    git = "https://github.com/flang-compiler/classic-flang-llvm-project.git"
    root_cmakelists_dir = "llvm"

    version("15.0.3", branch="release_15x")
    version("10.0.1", branch="release_100")

    depends_on('cmake@3.24', type='build')

    def cmake_args(self):
        gcc_libs = "/usr/local/software/skylake/Stages/2023/software/GCCcore/11.3.0/lib64"
        cache_path = os.path.join(self.stage.source_path, "caches", "Release-shared.cmake")
        args = [
                '-DLLVM_ENABLE_CLASSIC_FLANG=ON',
                self.define('LLVM_ENABLE_PROJECTS', 'clang'),
                '-DLLVM_TARGETS_TO_BUILD=X86',
                '-DFLANG_BUILD_NEW_DRIVER=OFF',
                '-DCMAKE_BUILD_TYPE=Release',
                self.define('CMAKE_INSTALL_RPATH', gcc_libs),
                ]
        return args

    def setup_run_environment(self, env):
        # Ideally we would ask user to use CMake integration, and not "pollute"
        # these common flags. This is only useful for instrumentation.
        env.prepend_path("LD_LIBRARY_PATH", self.prefix.lib)
        env.prepend_path("LIBRARY_PATH", self.prefix.lib)
