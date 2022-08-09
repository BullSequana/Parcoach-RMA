#!/bin/bash

ret_ok=0
ret_fail=0

path=$(readlink -f "${BASH_SOURCE:-$0}")
dirpath=$(dirname ${path})

for filename in $(ls ${dirpath}/bin/*)
do
    rc=0;
    if [[ "$filename" == *"bin/rr_"* ]]
    then
        nprocs=3
    else
        nprocs=2
    fi
    mpirun -np $nprocs --oversubscribe $filename > /dev/null 2>&1 || rc="$?"

    if (( "$rc" == 0 )); then
         ret=OK
         ret_ok=$(($ret_ok + 1))
    elif (( "$rc" == 124 || "$rc" == 137 )); then
         ret=TIMEOUT
    else
         ret=FAIL
         ret_fail=$(($ret_fail + 1))
    fi

    echo "$filename: $ret"
done

echo "OK: $ret_ok, expected: 13"
echo "FAIL: $ret_fail, expected: 18"
