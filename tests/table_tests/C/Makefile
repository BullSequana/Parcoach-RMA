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


CC=mpicc

DIRNAME=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
BINARIES=$(DIRNAME)/bin

SRC_TEST= $(wildcard $(DIRNAME)/local_local/*.c) \
          $(wildcard $(DIRNAME)/remote_local/*.c) \
          $(wildcard $(DIRNAME)/remote_remote/*.c) \

OBJ_TEST=$(SRC_TEST:.c=.o)
EXEC_TEST=$(SRC_TEST:.c=)

INSTALL_PREFIX?=$(DIRNAME)/build

all: $(EXEC_TEST)

$(EXEC_TEST): $(OBJ_TEST)
	mkdir -p $(DIRNAME)/bin
	$(CC) -o $(DIRNAME)/bin/$(shell basename $@) $@.o

%.o: %.c
	$(CC) -g -o $@ -c $<

clean:
	\rm -rf $(DIRNAME)/$(EXEC_TEST) \
			$(DIRNAME)/local_local/*.o \
			$(DIRNAME)/remote_local/*.o \
			$(DIRNAME)/remote_remote/*.o \
			$(DIRNAME)/bin
