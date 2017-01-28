#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "helper.h"
#include "loader.h"
#include "corank.h"
#include "merge_sequential.h"

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

  int i, j, x;
  i = j = x = 0;
  while(i < sample.size1 && j < sample.size2) {
	  if(sample.array1[i] < sample.array2[j]) {
		output[x++] = sample.array1[i++];  
	  } else {
		output[x++] = sample.array2[j++];
	  }
  }
  while(i < sample.size1) {
	  output[x++] = sample.array1[i++]; 
  }
  while(j < sample.size2) {
	  output[x++] = sample.array2[j++]; 
  }
  
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
