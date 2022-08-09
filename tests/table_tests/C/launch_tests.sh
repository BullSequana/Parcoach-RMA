#!/bin/bash

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
