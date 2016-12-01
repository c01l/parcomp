CC=gcc
CFLAGS=-O3 -lrt

DEPS=loader.o corank.o logger.o

BUILDDIR=build

all: sequential openmp cilk mpi

sequential: $(DEPS) merge_sequential.o
	gcc $^ -o $@

openmp: 
	echo "Not implemented yet!"
	
cilk:
	echo "Not implemented yet!"

mpi:
	echo "Not implemented yet!"
	
clean:
	rm -f $(DEPS) *.o sequential corank

corank: $(DEPS) corank_test.o
	gcc $^ -o $@
	./corank
