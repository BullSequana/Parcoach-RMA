#!/bin/bash
#!

######################################################################################################################
# for running this run script you can use the following syntax
#
# cat run_job.sh | sed "s/PROCS/4/" | bsub
# 4 represents the number of processes to use
#
# or you can use the following syntax for a bundle of submissions
#
# for i in 1 2 4 8 16 32 64 128 256 512 1024; do cat run_job.sh | sed "s/PROCS/$i/" | bsub; done
# the command will submit the run script with 1,2,4,8,16,32,64,128,256,512,1024 processes (a total of 11 submissions)
######################################################################################################################

PROCS=64
total_procs=$PROCS
nodes=16

#echo
#echo "GLOBAL DOMAIN MFS LIKE (16M grid points)"
#echo

ljpi_mfs=871
ljpj_mfs=253

lcpux=`echo "val=l(sqrt($total_procs*($ljpi_mfs-3)/($ljpj_mfs-3)))/l(2)+0.5 ; scale=0; 2^(val/1)"| bc -l`
lcpuy=$((total_procs/ lcpux))
if [ $lcpuy = 0 ]; then
    lcpuy=1
    lcpux=$total_procs
fi

jpi=$(( (ljpi_mfs + lcpux - 3) / lcpux ))
jpj=$(( (ljpj_mfs + lcpuy - 3) / lcpuy ))

#export JPK=72

#echo
#echo "GLOBAL DOMAIN: BIGGEST (600M grid points)"
#echo

#ljpi_big=5760
#ljpj_big=1440
#
#lcpux=`echo "val=l(sqrt($total_procs*($ljpi_big-3)/($ljpj_big-3)))/l(2)+0.5 ; scale=0; 2^(val/1)"| bc -l`
#lcpuy=$((total_procs / lcpux))
#if [ $lcpuy = 0 ]; then
#    lcpuy=1
#    lcpux=$total_procs
#fi
#
#jpi=$(( (ljpi_big + lcpux - 3) / lcpux ))
#jpj=$(( (ljpj_big + lcpuy - 3) / lcpuy ))

#export JPK=32
if [ ! -f script_launch.sh ]
then
    touch script_launch.sh
    chmod ugo+x script_launch.sh
fi
cat << EOF > script_launch.sh
#!/bin/bash
export OMPI_MCA_osc=ucx
export OMPI_MCA_btl=^vader,openib,uct
export OMPI_MCA_pml=ucx
export OMPI_MCA_coll=basic,libnbc,tuned
export PROC_X=${lcpux}
export PROC_Y=${lcpuy}
export JPI=${jpi}
export JPJ=${jpj}
export JPK=32
export CPU_PER_TASK=1
ulimit -s 65536
./tra_adv_test_rma_put
EOF

#./tra_adv_test_rma_put >> res14 2>&1
time srun -x pise13 -N $nodes -n $PROCS -u --cpus-per-task=5 --ntasks-per-node=$(($PROCS / $nodes)) --exclusive -J traadv_kernel -p rome7402 ./script_launch.sh

