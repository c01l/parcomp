#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "helper.h"
#include "loader.h"
#include "corank.h"
#include "merge_sequential.h"
#include <mpi.h>

int IS_LOGGING = 0;

int mpi_merge(struct merge_sample *sample, INPUTTYPE *output, int *argc, char ***argv);

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
  int rank = mpi_merge(&sample, output, &argc, &argv);
  if ( rank > 0) return 0;
  clock_gettime(CLOCK_REALTIME, &endtime);

  printTimeDiff(starttime, endtime);

  if(IS_LOGGING) echoArrayExt(output, n, "result-array");
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


int mpi_merge(struct merge_sample *sample, INPUTTYPE *output, int *argc, char ***argv) {
  MPI_Init(argc,argv);
  int TAG = 100;  // passing messages requires a fixed "tag" value; the value does not matter
  int mpiRank, mpiSize, mpiNameLength;
  MPI_Comm_size(MPI_COMM_WORLD, &mpiSize); //total number of processes -> mpiSize
  MPI_Comm_rank(MPI_COMM_WORLD, &mpiRank); //process number -> mpiRank
  char mpiName[MPI_MAX_PROCESSOR_NAME];
  MPI_Get_processor_name(mpiName,&mpiNameLength); // name of processor -> mpiName
  int i;
  MPI_Status mpiStatus;
  int len = sample->size1 + sample->size2;
  
  int numberOfElements = (mpiRank == mpiSize - 1 && (len % mpiSize) )? (len % mpiSize) : (len / mpiSize);
  int range1 = mpiRank * len / mpiSize;
  int range2 = (mpiRank + 1) * len / mpiSize;

  // 1) every process merges their parts and sends it to process 0
  if (mpiRank == 0) {
    // process 0 calculates directly into *output
    mergeSeq(sample, output, range1, range2, 0);
  } else {
    // the rest stores the merge result in *singleMergeOutput and sends it to 0
    INPUTTYPE *singleMergeOutput = malloc(sizeof(INPUTTYPE) * numberOfElements);
    mergeSeq(sample, singleMergeOutput, range1, range2, 0);
    int dest = 0;
    MPI_Send(singleMergeOutput, numberOfElements, MPI_INT, dest, TAG, MPI_COMM_WORLD);
    free(singleMergeOutput);
  }

  // 2) process #0 creates the outcome. therefore it listens to the rest for incoming messages
  if (mpiRank == 0) {
    MPI_Request *recv_request = malloc(sizeof(MPI_Request) * (mpiSize - 1));
    // for every other process (\w rank > 0) call a non blocking receiver
    for(i = 1; i < mpiSize; i++) {
      int numberOfReceivingElements = (i == mpiSize - 1 && (len%mpiSize)) ? (len % mpiSize) : (len / mpiSize);
      int offset = len / mpiSize;
      // put the received elements directly in the output array
      MPI_Irecv(output + (i *offset), numberOfReceivingElements, MPI_INT, i, TAG, MPI_COMM_WORLD, recv_request + (i - 1));
    }
    // wait until a message has been received from every other process 
    for(i = 1; i < mpiSize; i++) {
      MPI_Wait(recv_request + (i - 1), &mpiStatus);
    }
    free(recv_request);
  }
  MPI_Finalize();
  return mpiRank;
}

