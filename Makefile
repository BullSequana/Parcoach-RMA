#global variables
CC?=clang
CXX?=clang++
MPICC=mpicc
SRCDIR=./src
HEADDIR=./include
BUILDDIR=./build
CFLAGS=-Wall -fPIC -pthread -Wno-int-to-pointer-cast
CMAKE_OPTIONS=
LDFLAGS=
SONAR_OUTPUT=

DEBUG?=0
COVERAGE?=0
SONAR?=0

ifeq (${DEBUG},1)
  CFLAGS += -D__DEBUG
endif

ifeq (${COVERAGE},1)
  CFLAGS += -g --coverage
  LDFLAGS += -lgcov --coverage
  CMAKE_OPTIONS += -DCOVERAGE=ON
endif

ifeq (${SONAR},1)
  SONAR_OUTPUT += --sonar coverage.xml
endif

PREFIX?=$(BUILDDIR)

LIBDIR=$(PREFIX)/lib
INCDIR=$(PREFIX)/include

#Configuration

CORESRC=./src/core/interval.o \
        ./src/core/interval_list.o \
        ./src/core/interval_avl_tree.o \
        ./src/core/notif_avl_tree.o \
        $(SRCDIR)/core/rma_analyzer.o \
        $(SRCDIR)/dynamic/rma_analyzer_mpi_c_overload.o

all: core dynamic static

core: $(CORESRC)

dynamic:
	 mkdir -p $(LIBDIR)
	 $(CC) $(TARGET2) -Wl,-soname,librma_analyzer_dyn.so.1.0 -o $(LIBDIR)/librma_analyzer_dyn.so.1.0 -shared $(LDFLAGS)
	 cd $(LIBDIR) && ln -sf librma_analyzer_dyn.so.1.0 librma_analyzer_dyn.so

static:
	 cd $(SRCDIR)/static && mkdir -p build
	 cd $(SRCDIR)/static/build/ && cmake ../ -DCMAKE_C_COMPILER=$(CC) -DCMAKE_CXX_COMPILER=$(CXX) $(CMAKE_OPTIONS) && make -j2

install: dynamic static
	 mkdir -p $(INCDIR)
	 mkdir -p $(LIBDIR)
	 cp $(HEADDIR)/rma_analyzer.h $(INCDIR)
	 cp $(SRCDIR)/static/build/src/parcoachRMA.so $(LIBDIR)
	 cp $(SRCDIR)/static/build/lib/librma_analyzer.a $(LIBDIR)

%.o: %.c
	 @echo "Compilation of object file "$<
	$(MPICC) $(CFLAGS) -I$(HEADDIR) -c $< -o $@

testbuild:
	@echo "Building tests"
	make -f tests/table_tests/C/Makefile.llvm COVERAGE=$(COVERAGE)

testclean:
	@echo "Cleaning tests"
	make -f tests/table_tests/C/Makefile.llvm clean
	rm -f sonar.xml

testrun:
	@echo "Launching tests"
	./tests/table_tests/C/launch_tests_llvm.sh

test: testclean testbuild testrun

gcovr:
	@echo "Outputting gcovr coverage results"
	gcovr -v -r . -e src/dynamic -e src/static/build -e src/static/tests -e tests/ -e src/core/interval.c $(SONAR_OUTPUT)

clean:
	@echo "Deleting temporary files"
	rm -rf $(CORESRC) $(SRCDIR)/static/build/*
	rm -rf $(BUILDDIR)/*
	rm -rf $(SRCDIR)/core/*.gcno $(SRCDIR)/core/*.gcda
	rm -rf $(SRCDIR)/dynamic/*.gcno $(SRCDIR)/dynamic/*.gcda

cleanall: clean testclean
