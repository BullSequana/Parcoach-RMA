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


"""
EasyBuild support for Classic Clang compiler toolchain.

"""
from easybuild.toolchains.compiler.cclang import CClang
from easybuild.tools.toolchain.toolchain import SYSTEM_TOOLCHAIN_NAME


TC_CONSTANT_CLASSICCLANG = "ClassicClang"


class ClassicClang(CClang):
    """Compiler toolchain with Classic Clang compilers."""
    NAME = 'ClassicClang'
    COMPILER_MODULE_NAME = [NAME]
    COMPILER_FAMILY = TC_CONSTANT_CLASSICCLANG
    SUBTOOLCHAIN = SYSTEM_TOOLCHAIN_NAME
