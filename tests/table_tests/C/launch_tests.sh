#!/bin/bash
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



ret_ok=0
ret_fail=0

for elt in $(ls bin/*)
do
    rc=0;
    prefix=$(echo $elt | cut -d'_' -f1)
    if [[ $prefix == "bin/rr" ]]
    then
        nprocs=3
    else
        nprocs=2
    fi
    LD_PRELOAD=../../../build/lib/librma_analyzer_dyn.so mpirun -np $nprocs --oversubscribe $elt > /dev/null 2>&1 || rc="$?"

    if (( "$rc" == 0 )); then
         ret=OK
         ret_ok=$(($ret_ok + 1))
    elif (( "$rc" == 124 || "$rc" == 137 )); then
         ret=TIMEOUT
    else
         ret=FAIL
         ret_fail=$(($ret_fail + 1))
    fi

    echo "$elt: $ret"
done

echo "OK: $ret_ok, expected: 22"
echo "FAIL: $ret_fail, expected: 9"
