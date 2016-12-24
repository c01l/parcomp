#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include "openmp.h"
#include "helper.h"
#include "loader.h"
#include "corank.h"


int main(int argc, char *args[]) {
  struct merge_sample sample; 
  int dynamic_sample = 0;
  int a[] = { 1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
  int b[] = { 1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15};
  if(argc != 2) {
    // printf("useage: %s <samplefile> \n", args[0]);
    // printf("but right now just give a toss and take a sample sample.");
    sample.array1 = a;
    sample.array2 = b;
    sample.size1 = sizeof(a)/sizeof(a[0]);
    sample.size2 = sizeof(b)/sizeof(b[0]);
  } else {
    dynamic_sample = 1;
    //load it
	
	int loadErr = loadsample(args[1], &sample);
	if(loadErr != 0) {
		char* error;
		switch(loadErr) {
			case 1: error = "File could not be read!"; break;
			case 2: error = "Fileformat is wrong."; break;
			default: error = "An unknown error occured!"; break;
		}
		(void) fprintf(stderr, "Failed to load data: %s\n", error);
		exit(1);
	}
  }

  printf("Array 1: \n");
  echoArray(sample.array1, sample.size1);
  printf("Array 2: \n");
  echoArray(sample.array2, sample.size2);

  int n = sample.size1 + sample.size2;
  INPUTTYPE *output = malloc(sizeof(INPUTTYPE) * n);
  
  // set up timer:
  struct timespec starttime, endtime;
  clock_gettime(CLOCK_REALTIME, &starttime);
  openmp_merge(&sample, output);
  clock_gettime(CLOCK_REALTIME, &endtime);

  printTimeDiff(starttime, endtime);

  printf("\nresult:\n");
  echoArray(output, n);
  
  free(output);
  if(dynamic_sample) {
    freesample(&sample);
  }
  return 0;
}



void openmp_merge(struct merge_sample *sample, INPUTTYPE *output) {
  int max = omp_get_max_threads();
  int threads = 4;
  int len = sample->size1 + sample->size2;

  //private:
  int id, range1, range2;
  struct pair_of_coranks coranks1, coranks2;
  struct merge_sample merge_payload;

  if(threads < max) {
    if (threads < 1) threads = 1;
    omp_set_num_threads(threads);
  } else {
    threads = max;
  }

  printf("Number of threads (maximum): %i (%i)\n", threads, max);
  if(threads < 0){
    printf("max is < 0... not so good. Quit function\n");
    return;
  }

#pragma omp parallel num_threads(threads) private(id, coranks1, coranks2, range1, range2, merge_payload)
  {
    id = omp_get_thread_num();
    range1 = id*len/threads;
    range2 = (id+1)*len/threads;
    merge_log("Thread %i/%i just started with range %i - %i\n", id, threads,range1, range2);
    coranks1 = corank(range1, sample->array1, sample->size1, sample->array2,sample->size2);
    coranks2 = corank(range2, sample->array1, sample->size1, sample->array2,sample->size2);
    merge_log("corank1 #%d: (%d,%d)\n", id, coranks1.corank_A, coranks1.corank_B);
    merge_log("corank2 #%d: (%d,%d)\n", id, coranks2.corank_A, coranks2.corank_B);
    merge_payload.array1 = sample->array1 + coranks1.corank_A;
    merge_payload.size1 = coranks2.corank_A - coranks1.corank_A;
    merge_payload.array2 = sample->array2 + coranks1.corank_B;
    merge_payload.size2 = coranks2.corank_B - coranks1.corank_B;
    merge(&merge_payload, output, range1, range2);
    merge_log("%d result :", id);
    echoArray(output + range1, range2-range1);
  }
  merge_log("done\n");
  
}
