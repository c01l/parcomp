CC=gcc
CFLAGS=-O3 -lrt

DEPS=loader.o corank.o logger.o helper.o
TESTS=corank_test.o

BUILDDIR=build

all: sequential openmp cilk mpi

sequential: $(DEPS) merge_sequential.o
	$(CC) $^ -o $@

openmp: $(DEPS) openmp.o
	gcc -Wall -fopenmp -o omp.out -O3 openmp.c loader.c helper.c corank.c merge_sequential.c
	
cilk:
	echo "Not implemented yet!"

mpi: 
	echo "Not implemented yet!"
	
clean:
	rm -f $(DEPS) *.o sequential corank

test: $(DEPS) $(TESTS)
	$(CC) $^ -o $@
	./$@
