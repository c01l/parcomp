#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include "helper.h"
#include "loader.h"
#include "corank.h"
#include "merge_sequential.h"
#include "def.h"

void openmp_merge(struct merge_sample *sample, INPUTTYPE *output);


int main(int argc, char *argv[]) {
  struct merge_sample sample; 
  
  int errorCode = handleArguments(argc, argv, &sample);
  if(errorCode != 0) {
	  exit(errorCode);
  }
  
  
  printf("Array 1: \n");
  if(sample.size1 > 30) {
	  echoArray(sample.array1, 30);
	  printf("Output truncated because it is too large. (size=%d)\n", sample.size1);
  } else {
	  echoArray(sample.array1, sample.size1);
  }
  printf("Array 2: \n");
  if(sample.size2 > 30) {
	  echoArray(sample.array2, 30);
	  printf("Output truncated because it is too large. (size=%d)\n", sample.size2);
  } else {
	  echoArray(sample.array2, sample.size2);
  }
  
  printf("\n");

  int n = sample.size1 + sample.size2;
  INPUTTYPE *output = malloc(sizeof(INPUTTYPE) * n);
  
  // set up timer:
  struct timespec starttime, endtime;
  clock_gettime(CLOCK_REALTIME, &starttime);
  openmp_merge(&sample, output);
  clock_gettime(CLOCK_REALTIME, &endtime);

  printTimeDiff(starttime, endtime);

  printf("\nchecking result-array: ");
  if(checkSorted(output, n)==1) {
      printf("Correct\n");
  } else {
	  printf("Failed\n");
  }
  
  free(output);
  freesample(&sample);
  
  return 0;
}



void openmp_merge(struct merge_sample *sample, INPUTTYPE *output) {
  int threads = omp_get_max_threads();
  int len = sample->size1 + sample->size2;

  //private:
  int id, range1, range2;
  struct pair_of_coranks coranks1, coranks2;
  struct merge_sample merge_payload;
  


  printf("Number of threads (maximum): %i\n", threads);
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
    if(LOGGING_ACTIVE) echoArray(output + range1, range2-range1);
  }
  merge_log("done\n");
  
}
