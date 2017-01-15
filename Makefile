CC=gcc
CFLAGS=-O3 -lrt -Wall

DEPS=loader.o corank.o helper.o
TESTS=corank_test.o

BUILDDIR=build

all: sequential openmp cilk mpi

sequential: $(DEPS) merge_sequential.o
	$(CC) $^ -o $@

openmp: openmp.c openmp.h $(DEPS) merge_sequential.o
	gcc -fopenmp -o $@ $(DEPS) merge_sequential.o openmp.c
	
cilk: $(DEPS) cilk.c merge_sequential.o
	gcc -fcilkplus -lcilkrts -o $@ $^

mpi: $(DEPS) mpi.c merge_sequential.o
	mpicc -o mpi.out $^
clean:
	rm -f $(DEPS) *.o sequential corank openmp cilk

test: $(DEPS) $(TESTS)
	$(CC) $^ -o $@
	./$@
