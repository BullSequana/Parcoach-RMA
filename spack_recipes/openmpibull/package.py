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

class Openmpibull(AutotoolsPackage):
    """An open source Message Passing Interface implementation.

    The Open MPI Project is an open source Message Passing Interface
    implementation that is developed and maintained by a consortium
    of academic, research, and industry partners. Open MPI is
    therefore able to combine the expertise, technologies, and
    resources from all across the High Performance Computing
    community in order to build the best MPI library available.
    Open MPI offers advantages for system and software vendors,
    application developers and computer science researchers.
    """

    homepage = "https://www.open-mpi.org"
    git = "https://github.com/BullSequana/OpenMPI5"

    version('5.0.2', branch='v5.0.2_Bull', submodules=True)

    depends_on("autoconf", type="build")
    depends_on("automake", type="build")
    depends_on("libtool", type="build")

    def configure_args(self):
        args = []

        args.append('--enable-mpi-ext=notified_rma')
        args.append('--with-pmix=internal')
        args.append('--enable-mpirun-prefix-by-default')
        args.append('--with-cma')
        args.append('--with-lustre=no')
        args.append('--with-verbs=no')
        args.append('--with-hcoll=no')
        args.append('--with-libevent=internal')
        args.append('--with-hwloc=internal')
        args.append('--without-cuda')
        args.append('--enable-mem-debug=no')
        args.append('--enable-mem-profile=no')
        args.append('--enable-debug=no')
        args.append('--enable-debug-symbols=no')
        args.append('--enable-memchecker=no')
        args.append('--enable-orterun-prefix-by-default=yes')
        args.append('--enable-wrapper-rpath=no')
        args.append('--enable-wrapper-runpath=no')
        args.append('--enable-mca-no-build=crs,snapc,pml-crcpw,pml-v,vprotocol,crcp,btl-usnic,btl-uct,btl-openib,coll-ml')
        args.append('--enable-io-romio=yes')
        args.append('--with-io-romio-flags=--with-file-system=ufs+nfs')

        return args

    def autoreconf(self, spec, prefix):
        os.system('./autogen.pl')
