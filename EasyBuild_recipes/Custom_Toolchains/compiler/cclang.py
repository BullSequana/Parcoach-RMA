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
Support for CClang as toolchain compiler.
"""

import easybuild.tools.systemtools as systemtools
from easybuild.tools.build_log import EasyBuildError
from easybuild.tools.toolchain.compiler import Compiler


TC_CONSTANT_CCLANG = "CClang"


class CClang(Compiler):
    """CClang compiler class"""

    COMPILER_MODULE_NAME = ['CClang']
    COMPILER_FAMILY = TC_CONSTANT_CCLANG

    COMPILER_UNIQUE_OPTS = {
        'loop-vectorize': (False, "Loop vectorization"),
        'basic-block-vectorize': (False, "Basic block vectorization"),
    }
    COMPILER_UNIQUE_OPTION_MAP = {
        'unroll': 'funroll-loops',
        'loop-vectorize': ['fvectorize'],
        'basic-block-vectorize': ['fslp-vectorize'],
        'optarch': 'march=native',
        # CClang's options do not map well onto these precision modes.  The flags enable and disable certain classes of
        # optimizations.
        #
        # -fassociative-math: allow re-association of operands in series of floating-point operations, violates the
        # ISO C and C++ language standard by possibly changing computation result.
        # -freciprocal-math: allow optimizations to use the reciprocal of an argument rather than perform division.
        # -fsigned-zeros: do not allow optimizations to treat the sign of a zero argument or result as insignificant.
        # -fhonor-infinities: disallow optimizations to assume that arguments and results are not +/- Infs.
        # -fhonor-nans: disallow optimizations to assume that arguments and results are not +/- NaNs.
        # -ffinite-math-only: allow optimizations for floating-point arithmetic that assume that arguments and results
        # are not NaNs or +-Infs (equivalent to -fno-honor-nans -fno-honor-infinities)
        # -funsafe-math-optimizations: allow unsafe math optimizations (implies -fassociative-math, -fno-signed-zeros,
        # -freciprocal-math).
        # -ffast-math: an umbrella flag that enables all optimizations listed above, provides preprocessor macro
        # __FAST_MATH__.
        #
        # Using -fno-fast-math is equivalent to disabling all individual optimizations, see
        # http://llvm.org/viewvc/llvm-project/cfe/trunk/lib/Driver/Tools.cpp?view=markup (lines 2100 and following)
        #
        # 'strict', 'precise' and 'defaultprec' are all ISO C++ and IEEE complaint, but we explicitly specify details
        # flags for strict and precise for robustness against future changes.
        'strict': ['fno-fast-math'],
        'precise': ['fno-unsafe-math-optimizations'],
        'defaultprec': [],
        'loose': ['ffast-math', 'fno-unsafe-math-optimizations'],
        'veryloose': ['ffast-math'],
        'vectorize': {False: 'fno-vectorize', True: 'fvectorize'},
    }

    # used when 'optarch' toolchain option is enabled (and --optarch is not specified)
    COMPILER_OPTIMAL_ARCHITECTURE_OPTION = {
        (systemtools.POWER, systemtools.POWER): 'mcpu=native',  # no support for march=native on POWER
        (systemtools.POWER, systemtools.POWER_LE): 'mcpu=native',  # no support for march=native on POWER
        (systemtools.X86_64, systemtools.AMD): 'march=native',
        (systemtools.X86_64, systemtools.INTEL): 'march=native',
    }
    # used with --optarch=GENERIC
    COMPILER_GENERIC_OPTION = {
        (systemtools.X86_64, systemtools.AMD): 'march=x86-64 -mtune=generic',
        (systemtools.X86_64, systemtools.INTEL): 'march=x86-64 -mtune=generic',
    }

    COMPILER_CC = 'clang'
    COMPILER_CXX = 'clang++'
    COMPILER_C_UNIQUE_FLAGS = []
    COMPILER_F77 = 'flang'
    COMPILER_F90 = 'flang'
    COMPILER_FC = 'flang'
    COMPILER_F_UNIQUE_FLAGS = []

    LIB_MULTITHREAD = ['pthread']
    LIB_MATH = ['m']

    def _set_compiler_vars(self):
        """Set compiler variables."""
        super(CClang, self)._set_compiler_vars()

        if self.options.get('32bit', None):
            raise EasyBuildError("_set_compiler_vars: 32bit set, but no support yet for 32bit CClang in EasyBuild")
