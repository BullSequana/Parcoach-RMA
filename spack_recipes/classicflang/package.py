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
import shutil
import sys


class Classicflang(CMakePackage):
    """ClassicFlang compiler (previously F18)"""

    homepage = "https://github.com/flang-compiler"
    git = "https://github.com/flang-compiler/flang.git"

    version("15.0.3", branch="master")
    version("10.0.1", branch="legacy")

    depends_on('cmake@3.24', type='build')
    depends_on('classicclang@15.0.3', when="@15.0.3")
    depends_on('classicclang@10.0.1', when="@10.0.1")

    depends_on('libpgmath@15.0.3', when="@15.0.3")
    depends_on('libpgmath@10.0.1', when="@10.0.1")

    def cmake_args(self):
        cache_path = os.path.join(self.stage.source_path, "caches", "Release-shared.cmake")
        args = [
                '-DLLVM_TARGETS_TO_BUILD=X86',
                '-DCMAKE_BUILD_TYPE=Release',
                '-DCMAKE_C_COMPILER=clang',
                '-DCMAKE_CXX_COMPILER=clang++',
                '-DCMAKE_Fortran_COMPILER=flang',
                '-DCMAKE_Fortran_COMPILER_ID=Flang',
                '-DFLANG_LLVM_EXTENSIONS=ON'
                ]
        return args

    @run_after('install')
    def copy_to_cclang(self):
        copied = False
        flang_bindir = os.path.join(self.prefix, 'bin')
        flang_incdir = os.path.join(self.prefix, 'include')
        flang_libdir = os.path.join(self.prefix, 'lib')
        for installed_spec in spack.environment.installed_specs():
            if (installed_spec.name == "classicclang" and self.version == installed_spec.version):
                clang_bindir = os.path.join(installed_spec.prefix, 'bin')
                clang_incdir = os.path.join(installed_spec.prefix, 'include')
                clang_libdir = os.path.join(installed_spec.prefix, 'lib')
                for f in os.listdir(flang_bindir):
                    filepath = os.path.join(flang_bindir, f)
                    if (os.path.isfile(filepath) and os.path.isdir(clang_bindir)):
                        shutil.copy2(filepath, clang_bindir)
                        print("Copied flang bin file to ", clang_bindir)
                        copied = True
                for f in os.listdir(flang_incdir):
                    filepath = os.path.join(flang_incdir, f)
                    if (os.path.isfile(filepath) and os.path.isdir(clang_incdir)):
                        shutil.copy2(filepath, clang_incdir)
                        print("Copied flang include file to ", clang_incdir)
                        copied = True
                for f in os.listdir(flang_libdir):
                    filepath = os.path.join(flang_libdir, f)
                    if (os.path.isfile(filepath) and os.path.isdir(clang_libdir)):
                        shutil.copy2(filepath, clang_libdir)
                        print("Copied flang lib file to ", clang_libdir)
                        copied = True

        if (not copied):
            sys.exit()
