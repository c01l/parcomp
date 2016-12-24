CC=gcc
CFLAGS=-O3 -lrt -Wall

DEPS=loader.o corank.o helper.o
TESTS=corank_test.o

BUILDDIR=build

all: sequential openmp cilk mpi

sequential: $(DEPS) merge_sequential.o
	$(CC) $^ -o $@

openmp: openmp.c openmp.h $(DEPS) merge_sequential.o
	gcc -fopenmp -o $@ -O3 $(DEPS) merge_sequential.o openmp.c
	
cilk:
	echo "Not implemented yet!"

mpi: 
	echo "Not implemented yet!"
	
clean:
	rm -f $(DEPS) *.o sequential corank openmp

test: $(DEPS) $(TESTS)
	$(CC) $^ -o $@
	./$@
