EXEC=tra_adv_test_rma_put
CC=mpifort
# Needed for GCC version >= 10
#CFLAGS=-fallow-argument-mismatch
LDFLAGS=-L/home_nfs/sergentm/workspace/softs/rma-analyzer/lib/ -lrma_analyzer

all: build run

build:
	$(CC) $(CFLAGS) $(EXEC).F90 -o $(EXEC)

run:
	./run_rma.sh

llvm:
	mpifort -o $(EXEC).bc -c -emit-llvm $(EXEC).F90
	opt -load ../../../lib/parcoachRMA.so -parcoach $(EXEC).bc -o $(EXEC)INSTR.bc
	mpifort -c $(EXEC)INSTR.bc
	mpifort -o $(EXEC) $(EXEC)INSTR.o $(LDFLAGS)

clean:
	rm -f $(EXEC) ./script_launch.sh *.bc *.o
