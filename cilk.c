#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "helper.h"
#include "loader.h"
#include "corank.h"
#include <cilk/cilk.h>

void cilk_merge(struct merge_sample *sample, INPUTTYPE *output);
void cilk_mergeRecursive(struct merge_sample *sample, INPUTTYPE *output, int range1, int range2);

/**
 * @brief Merges two arrays from a merge_sample to an output-array. It will only sort the elements in the given range. This method does not spawn any new workers.
 * @param sample the merge_sample you want to merge
 * @param output the output array you want the solution to be written to. (must be big enough for both arrays to fit in it)
 * @param range1 the start of the range you want to merge (inclusive)
 * @param range2 the end of the range you want to merge (inclusive)
 */
void cilk_mergeSeq(struct merge_sample *sample, INPUTTYPE *output, int range1, int range2);

int main(int argc, char** argv) {
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
  cilk_merge(&sample, output);
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



void cilk_merge(struct merge_sample *sample, INPUTTYPE *output) {
  int threads = 4;
  int len = sample->size1 + sample->size2;

  cilk_mergeRecursive(sample, output, 0, len - 1);
  
  merge_log("done\n");  
}

void cilk_mergeRecursive(struct merge_sample *sample, INPUTTYPE *output, int range1, int range2) {
	int size = range2 - range1 + 1;
	
	if(size < 1024) {
		merge_log("Starting sequential mode for [%d; %d] (size=%d)", range1, range2, size);
		cilk_mergeSeq(sample, output, range1, range2);
		return;
	}
	
	int middle = range1 + size / 2;
	
	merge_log("Splitting in [%d; %d] and [%d; %d]", range1, middle - 1, middle, range2);
	
	cilk_spawn cilk_mergeRecursive(sample, output, range1, middle - 1);
	cilk_spawn cilk_mergeRecursive(sample, output, middle, range2);
}

void cilk_mergeSeq(struct merge_sample *sample, INPUTTYPE *output, int range1, int range2) {
	struct merge_sample merge_payload;
	struct pair_of_coranks coranks1, coranks2;
    
	coranks1 = corank(range1, sample->array1, sample->size1, sample->array2,sample->size2);
    coranks2 = corank(range2 + 1, sample->array1, sample->size1, sample->array2,sample->size2);
    merge_log("corank1: (%d,%d)\n", coranks1.corank_A, coranks1.corank_B);
    merge_log("corank2: (%d,%d)\n", coranks2.corank_A, coranks2.corank_B);
    
	merge_payload.array1 = sample->array1 + coranks1.corank_A;
    merge_payload.size1 = coranks2.corank_A - coranks1.corank_A;
    merge_payload.array2 = sample->array2 + coranks1.corank_B;
    merge_payload.size2 = coranks2.corank_B - coranks1.corank_B;
    
	merge(&merge_payload, output, range1, range2 + 1);
    
	//merge_log("%d result :", id);
    if(LOGGING_ACTIVE) echoArray(output + range1, range2-range1);
}