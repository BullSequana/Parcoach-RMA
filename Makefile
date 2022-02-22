#global variables
CC=mpicc
SRCDIR=./src
HEADDIR=./include
BUILDDIR=./build
CFLAGS=-g -Wall -fPIC -pthread -Wno-int-to-pointer-cast

DEBUG?=0

ifeq (${DEBUG},1)
  CFLAGS += -D__DEBUG
endif

PREFIX?=$(BUILDDIR)

LIBDIR=$(PREFIX)/lib
INCDIR=$(PREFIX)/include

#Configuration

CORESRC=./src/core/interval.o \
        ./src/core/interval_list.o \
        ./src/core/interval_tree.o \
        $(SRCDIR)/core/rma_analyzer.o \
        $(SRCDIR)/dynamic/rma_analyzer_mpi_c_overload.o

all: core dynamic static

core: $(CORESRC)

dynamic:
	 mkdir -p $(LIBDIR)
	 $(CC) $(CFLAGS) $(TARGET2) -Wl,-soname,librma_analyzer_dyn.so.1.0 -o $(LIBDIR)/librma_analyzer_dyn.so.1.0 -shared
	 cd $(LIBDIR) && ln -s librma_analyzer_dyn.so.1.0 librma_analyzer_dyn.so

static:
	 cd $(SRCDIR)/static && mkdir -p build
	 cd $(SRCDIR)/static/build/ && CC=clang CXX=clang++ cmake ../ && make -j2

install: dynamic static
	 mkdir -p $(INCDIR)
	 mkdir -p $(LIBDIR)
	 cp $(HEADDIR)/rma_analyzer.h $(INCDIR)
	 cp $(SRCDIR)/static/build/src/parcoachRMA.so $(LIBDIR)
	 cp $(SRCDIR)/static/build/lib/librma_analyzer.a $(LIBDIR)

%.o: %.c
	 @echo "Compilation of object file "$<
	$(CC) $(CFLAGS) -I$(HEADDIR) -c $< -o $@

clean:
	@echo "Deleting temporary files"
	rm -rf $(CORESRC) $(SRCDIR)/static/build/*
	rm -rf $(BUILDDIR)/*
