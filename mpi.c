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

int getValueFromRank(int index, int rank, MPI_Win window) {
    MPI_Datatype type = MPI_INT;
    int count = 1, disp = 0, out;

    MPI_Win_create(NULL, 0, 1, MPI_INFO_NULL, MPI_COMM_WORLD,&window);

    MPI_Win_lock(MPI_LOCK_SHARED, rank, 0, window);
    printf("calling get with rank = %i, disp = %i, count = %i\n", rank, disp, count);
    MPI_Get(&out, count, type, rank, disp, count, type, window);
    MPI_Win_unlock(rank, window);
    MPI_Win_free(&window);
    return out;
}

int main(int argc, char** argv) {
  int TAG_sending_input_part1 = 100;
  int TAG_sending_input_part2 = 101;
  struct merge_sample sample; 
  
  // set up MPI variables
  MPI_Init(&argc,&argv);
  int mpiRank, mpiSize, mpiNameLength;
  MPI_Comm_size(MPI_COMM_WORLD, &mpiSize); //total number of processes -> mpiSize
  MPI_Comm_rank(MPI_COMM_WORLD, &mpiRank); //process number -> mpiRank
  char mpiName[MPI_MAX_PROCESSOR_NAME];
  MPI_Get_processor_name(mpiName,&mpiNameLength); // name of processor -> mpiName
  MPI_Win windows[mpiSize];

    // read input:
    int errorCode = handleArguments(argc, argv, &sample);
    if(errorCode != 0) {
	    exit(errorCode);
    }
  if(mpiRank == 0) {
    //print input arrays
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
    //split up array for rank 0
  
    int n = sample.size1 + sample.size2;
    // TODO: INPUTTYPE *output = malloc(sizeof(INPUTTYPE) * n);

    if(sample.size1 % mpiSize || sample.size2 % mpiSize) {
      printf("Rank#%i: At the moment the mpiSize must devide the length of the input arrays\n", mpiRank);
      MPI_Finalize();
      return 0;
    }
    int len1 = sample.size1/mpiSize;
    INPUTTYPE array1parts[mpiSize][len1];
    int len2 = sample.size2/mpiSize;
    INPUTTYPE array2parts[mpiSize][len2];
    int i;
    for(i = 0; i < sample.size1; i++) {
      array1parts[i/len1][i % len1] = sample.array1[i];
    }
    for(i = 0; i < sample.size2; i++) {
      array2parts[i/len2][i % len2] = sample.array2[i];
    }

    // send the parts to all the other ranks
    MPI_Request *send_requests = malloc(sizeof(MPI_Request)*(mpiSize - 1));
    for(i = 1; i < mpiSize; i++){
      printf("sending from %d to %d \n", mpiRank, i);
      MPI_Isend(array1parts[i], len1, MPI_INT, i, TAG_sending_input_part1,MPI_COMM_WORLD, send_requests + (i - 1));
      MPI_Isend(array2parts[i], len2, MPI_INT, i, TAG_sending_input_part2,MPI_COMM_WORLD, send_requests + (i - 1));
    }
    free(send_requests);

    int *input1 = array1parts[0];
    int *input2 = array2parts[0];
    char s[16];
    sprintf(s, "rank%i input1", mpiRank);
    echoArrayExt(input1, len1, s);
    sprintf(s, "rank%i input2", mpiRank);
    echoArrayExt(input2, len2, s);

    // open access window for other ranks
    printf("open window rank %i\n", mpiRank);
    int value = input1[0];
    MPI_Win_create(input1, sizeof(int), sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD,&windows[mpiRank]);

  } else {
    MPI_Status mpiStatus;
    int total_len1 = sample.size1;
    int total_len2 = sample.size2;
    // TODO: each process needs to know it's size :O
    if(total_len1 % mpiSize || total_len2 % mpiSize) {
      printf("Rank#%i: At the moment the mpiSize(%i) must devide the length of the input arrays(%i, %i)\n", mpiRank, mpiSize, sample.size1, sample.size2);
      MPI_Finalize();
      return 0;
    }
    // GET input from RANK 0
    int len1 = total_len1/mpiSize; 
    int len2 = total_len2/mpiSize; 
    printf("startet rank#%i, waiting for arrays of length %i and %i\n", mpiRank, len1, len2);
    int input1[len1];
    int *input2 = malloc(sizeof(INPUTTYPE) * len2);
    MPI_Recv(input1, len1, MPI_INT, 0, TAG_sending_input_part1, MPI_COMM_WORLD,&mpiStatus);
    MPI_Recv(input2, len2, MPI_INT, 0, TAG_sending_input_part2, MPI_COMM_WORLD,&mpiStatus);
    char s[16];
    sprintf(s, "rank%i input1", mpiRank);
    echoArrayExt(input1, len1, s);
    sprintf(s, "rank%i input2", mpiRank);
    echoArrayExt(input2, len2, s);

    // Get values from other ranks
    int other = -1;
    other = getValueFromRank(mpiRank, 0, windows[0]);
    
    printf("Rank #%i's laluee: %i\n", mpiRank, other); 

    free(input2);
  }


  /*
  int A[sample.size1/mpiSize], j;
  for(j = 0; j < sample.size1/mpiSize; j++) A[j] = -5;

  MPI_Win_fence(0, win1get);
  MPI_Win_fence(0, win2get);
  int value1 = 0;
  int value2 = 0;
  int element_index = 0;
  int next_process = (mpiRank + 1) % mpiSize;
  MPI_Get(input1, 1, MPI_INT, next_process, element_index, 1, MPI_INT, win1get);
  MPI_Get(input2, 1, MPI_INT, next_process, element_index, 1, MPI_INT, win2get);
  MPI_Win_fence(0, win1get);
  MPI_Win_fence(0, win2get);
  printf("Prozess #%i calculated %i and %i from %i\n", mpiRank, value1, value2, next_process);

  MPI_Win_free(&win1get);
  MPI_Win_free(&win2get);
  MPI_Win_free(&win1provide);
  MPI_Win_free(&win2provide);
  */
  printf("terminating rank %i \n", mpiRank);
  int i;
  //for(index = 0; index < mpiSize; index++){
  if(mpiRank == 0)
      MPI_Win_free(&windows[0]);
  //}
  MPI_Finalize();

     


/*
  struct timespec starttime, endtime; 
  // split the arrays
  
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
  */
  return 0;
}

/*
int mpi_merge(struct merge_sample *sample, INPUTTYPE *output, int *argc, char ***argv) {
  int TAG = 100;  // passing messages requires a fixed "tag" value; the value does not matter
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
*/
