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
	$(CC) -o $(DIRNAME)/bin/$(shell basename $@) $@.o $(LIBS)

%.o: %.c
	$(CC) -g -o $@ -c $<

clean:
	\rm -rf $(DIRNAME)/$(EXEC_TEST) \
			$(DIRNAME)/local_local/*.o \
			$(DIRNAME)/remote_local/*.o \
			$(DIRNAME)/remote_remote/*.o \
			$(DIRNAME)/bin
