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

from pprint import pprint
from spack.package import *
import os
import shutil
import sys


class Libpgmath(CMakePackage):
    """ClassicFlang compiler (previously F18)"""

    homepage = "https://github.com/flang-compiler"
    git = "https://github.com/flang-compiler/flang.git"

    version("15.0.3", branch="master")
    version("10.0.1", branch="legacy")

    depends_on('cmake@3.24', type='build')
    depends_on('classicclang@15.0.3', when="@15.0.3")
    depends_on('classicclang@10.0.1', when="@10.0.1")


    root_cmakelists_dir = 'runtime/libpgmath'

    def cmake_args(self):
        cache_path = os.path.join(self.stage.source_path, "caches", "Release-shared.cmake")
        args = [
                '-DCMAKE_BUILD_TYPE=Release',
                '-DCMAKE_C_COMPILER=clang',
                '-DCMAKE_CXX_COMPILER=clang++',
                '-DCMAKE_Fortran_COMPILER=flang',
                '-DCMAKE_Fortran_COMPILER_ID=Flang'
                ]
        return args

    @run_after('install')
    def copy_to_cclang(self):
        libpgmath_libdir = os.path.join(self.prefix, 'lib')
        #  repository = spack.repo.Repo("$spack/var/spack/repos/builtin")
        #clang_pkg = repository.get_pkg_class('classicclang@' + self.version.dotted.string)
        #  clang_pkg = repository.get_pkg_class('classicclang')
        copied = False
        for installed_spec in spack.environment.installed_specs():
            if (installed_spec.name == "classicclang" and self.version == installed_spec.version):
                clang_libdir = os.path.join(installed_spec.prefix, 'lib')
                for f in os.listdir(libpgmath_libdir):
                    filepath = os.path.join(libpgmath_libdir, f)
                    if (os.path.isfile(filepath) and os.path.isdir(clang_libdir)):
                        shutil.copy2(filepath, clang_libdir)
                        print("Copied libpgmath file to ", clang_libdir)
                        copied = True

        if (not copied):
            sys.exit()

                #  print(installed_spec.version)
                #  print(self.version)
                #  print(installed_spec.prefix)
        #  clang_prefix = clang_pkg.prefix + 'lib'
        #  print(clang_prefix)
        #  clang_libdir = os.path.join(clang_pkg.prefix, 'lib')
        #  for file in os.listdir(libpgmath_libdir):
        #      filepath = os.path.join(libpgmath_libdir, file)
        #      if (os.path.isfile(filepath)):
        #          shutil.copy2(filepath, clang_libdir)
