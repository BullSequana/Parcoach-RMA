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


EXEC=tra_adv_test_rma_put
CC=mpifort
# Needed for GCC version >= 10
#CFLAGS=-fallow-argument-mismatch
LDFLAGS=-L../../../build/lib/ -lrma_analyzer

all: build run

build:
	$(CC) $(CFLAGS) $(EXEC).F90 -o $(EXEC)

run:
	./run_rma.sh

llvm:
	mpifort -o $(EXEC).bc -c -emit-llvm $(EXEC).F90
	opt -load ../../../build/lib/parcoachRMA.so -parcoach $(EXEC).bc -o $(EXEC)INSTR.bc
	mpifort -c $(EXEC)INSTR.bc
	mpifort -o $(EXEC) $(EXEC)INSTR.o $(LDFLAGS)

clean:
	rm -f $(EXEC) ./script_launch.sh *.bc *.o
