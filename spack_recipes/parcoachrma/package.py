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


class Parcoachrma(Package):
    """Analysis tool for MPI-RMA programs"""

    homepage = "https://parcoach.github.io/index.html"
    git      = "https://github.com/BullSequana/Parcoach-RMA"

    version('main', branch='main')

    depends_on('openmpibull-classicclang')
    depends_on('classicflang@10.0.1')
    depends_on('classicclang@10.0.1')
    depends_on('cmake@3.24', type='build')

    def install(self, spec, prefix):
        os.environ['PREFIX'] = prefix
        make('install')
