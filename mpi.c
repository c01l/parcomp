#include <stdio.h>
#include <mpi.h>
#include <limits.h>
#include "loader.h"
#include "corank.h"

int mpilogging = 1;

int getBlocksize(int totalArrayLength, int numberOfProcesses) {
  return totalArrayLength / numberOfProcesses;
}

int getVal(int index, int total_length, MPI_Win win1, int proc, int thisRank, int *thisInput) {
  if(index < 0) return INT_MIN;
  int blocksize = getBlocksize(total_length, proc);
  int value = -1;
  int targetRank = index / blocksize;
  int d = index % blocksize;
    
  if(targetRank == thisRank) {
    return thisInput[d];
  }
  if(targetRank >= proc){
     return INT_MAX;
  }
  if(targetRank < 0) return INT_MIN;
  MPI_Win_lock(MPI_LOCK_SHARED, targetRank, 0, win1);
    MPI_Get(&value, 1, MPI_INT, targetRank, d, 1, MPI_INT, win1);
  MPI_Win_unlock(targetRank, win1);
  return value;
}


struct pair_of_coranks corank_mpi(int i, int rank, int sizeA, int sizeB, MPI_Win win1, MPI_Win win2,int mpiSize, int *A, int *B) {
  struct pair_of_coranks coranks;
  int j = min(i, sizeA);
  int k = (i - j);
  int j_low = max (0, (i- sizeA));
  int k_low = 0;
  int counter = 0;
  int A1, A2, B1, B2;

  while(counter < sizeA + sizeB) {
    A1 = getVal(j-1,sizeA, win1, mpiSize, rank, A);
    B1 = getVal(k, sizeB, win2, mpiSize, rank, B);
    A2 = getVal(j, sizeA, win1, mpiSize, rank, A);
    B2 = getVal(k-1, sizeA, win2, mpiSize, rank, B);
    if(j > 0 && k < sizeB && A1 > B1 ) {
      k_low = k;
      int diff = j - j_low;
      int delta = diff/2 + (diff&1);
      j = j - delta;
      k = k + delta;
    } else if (k>0 && j<sizeA && B2 >= A2){
      j_low = j;
      int diff = k - k_low;
      int delta = diff/2 + (diff&1);
      k = k - delta;
      j = j + delta;
    } else {
      coranks.corank_A = j;
      coranks.corank_B = k;
      break;
    }
    counter++;
  }
  return coranks;
}


int main(int argc, char** argv)
{
  int TAG_sending_input_part1 = 100;
  int TAG_sending_input_part2 = 101;
  int TAG_sending_merge_output = 102;
  struct merge_sample sample; 
  
  // set up MPI variables
  MPI_Init(&argc,&argv);
  int mpiRank, mpiSize, mpiNameLength, i;
  MPI_Comm_size(MPI_COMM_WORLD, &mpiSize); //total number of processes -> mpiSize
  MPI_Comm_rank(MPI_COMM_WORLD, &mpiRank); //process number -> mpiRank
  char mpiName[MPI_MAX_PROCESSOR_NAME];
  MPI_Get_processor_name(mpiName,&mpiNameLength); // name of processor -> mpiName
  MPI_Win winA, winB;
  int *A, *B;

  // read input:
  int errorCode = handleArguments(argc, argv, &sample);
  if(errorCode != 0) {
	  exit(errorCode);
  }
  //if(mpiRank == 0) {
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
    int sizeA = sample.size1;
    int sizeB = sample.size2;
    int len1 = sizeA/mpiSize;
    int len2 = sizeB/mpiSize;
    A = malloc(sizeof(INPUTTYPE) * len1);
    B = malloc(sizeof(INPUTTYPE) * len2);
    int len = sizeA + sizeB;
  if(mpiRank == 0) {
    INPUTTYPE array1parts[mpiSize][len1];
    INPUTTYPE array2parts[mpiSize][len2];
    for(i = 0; i < sample.size1; i++) {
      array1parts[i/len1][i % len1] = sample.array1[i];
    }
    for(i = 0; i < sample.size2; i++) {
      array2parts[i/len2][i % len2] = sample.array2[i];
    }

    // send the parts to all the other ranks
    MPI_Request *send_requests = malloc(sizeof(MPI_Request)*(mpiSize - 1));
    for(i = 1; i < mpiSize; i++){
      MPI_Isend(array1parts[i], len1, MPI_INT, i, TAG_sending_input_part1,MPI_COMM_WORLD, send_requests + (i - 1));
      MPI_Isend(array2parts[i], len2, MPI_INT, i, TAG_sending_input_part2,MPI_COMM_WORLD, send_requests + (i - 1));
    }
    free(send_requests);
    
    for(i = 0; i < len1; i++) A[i] = array1parts[0][i];
    for(i = 0; i < len2; i++) B[i] = array2parts[0][i];
  } else {
    MPI_Status mpiStatus;
    // TODO: each process needs to know it's size :O
    if(sizeA % mpiSize || sizeB % mpiSize) {
      printf("Rank#%i: At the moment the mpiSize(%i) must devide the length of the input arrays(%i, %i)\n", mpiRank, mpiSize, sample.size1, sample.size2);
      MPI_Finalize();
      return 0;
    }
    // GET input from RANK 0
    MPI_Recv(A, len1, MPI_INT, 0, TAG_sending_input_part1, MPI_COMM_WORLD,&mpiStatus);
    MPI_Recv(B, len2, MPI_INT, 0, TAG_sending_input_part2, MPI_COMM_WORLD,&mpiStatus);
  }
   

   MPI_Win_create(A, len1, sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD, &winA);
   MPI_Win_create(B, len2, sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD, &winB);
  

  // << CORANK
  struct pair_of_coranks corank1, corank2;
  int index1 = mpiRank*len/mpiSize;
  int index2 = (mpiRank + 1)*len/mpiSize;
  
  corank1 = corank_mpi(index1, mpiRank,sizeA,sizeB , winA, winB, mpiSize, A, B);
  corank2 = corank_mpi(index2, mpiRank,sizeA,sizeB , winA, winB, mpiSize, A, B);
  // CORANK>> */

  //<< LOCAL MERGE
  // get the entries
  int indexInput1 = corank1.corank_A, size1 = corank2.corank_A - corank1.corank_A;
  int array1[size1];
  int indexInput2 = corank1.corank_B, size2 = corank2.corank_B - corank1.corank_B;
  int array2[size2];
  for(i = 0; i < size1; i++, indexInput1++) {
    array1[i] = getVal(indexInput1, sizeA, winA, mpiSize, mpiRank, A); 
  }
  for(i = 0; i < size2; i++, indexInput2++) {
    array2[i] = getVal(indexInput2, sizeB, winB, mpiSize, mpiRank, B); 
  }

   MPI_Win_free(&winA);
   MPI_Win_free(&winB);
  // merge
  int *output = malloc(sizeof(int) * (size1 + size2));
  if(size1 == 0){
    output = array2;
  } else if (size2 == 0) {
    output = array1;
  } else {
    struct merge_sample merge_payload;
    merge_payload.array1 = array1;
    merge_payload.size1 = size1;
    merge_payload.array2 = array2;
    merge_payload.size2 = size2;
    merge(&merge_payload, output, 0, size1 + size2);
  }
  // MERGE >>

  int numberOfElements = (mpiRank == mpiSize - 1 && (len % mpiSize) ) ? (len % mpiSize) : (len / mpiSize);
  // << send parts to Rank 0:
  if(mpiRank == 0){
    int *result = malloc(sizeof(int) * len);
    for(i = 0; i< size1 + size2; i++) result[i] = output[i];
    MPI_Request *recv_request = malloc(sizeof(MPI_Request) * (mpiSize - 1));
    for(i = 1; i < mpiSize; i++) {
      int numberOfReceivingElements = (i == mpiSize - 1 && (len % mpiSize))? (len % mpiSize) : (len/mpiSize);
      int offset = len /mpiSize;
      MPI_Irecv(result + (i*offset), numberOfReceivingElements, MPI_INT, i, TAG_sending_merge_output, MPI_COMM_WORLD, recv_request + (i - 1));
    }
    // wait until all everything is received
    MPI_Status mpiStatus;
    for(i=1; i<mpiSize; i++) MPI_Wait(recv_request + (i - 1), &mpiStatus);
    free(recv_request);
    echoArray(result, len);
    free(result);
  } else {
    // the rest sends their merge-outputs to 0
    int dest = 0;
    MPI_Send(output, numberOfElements, MPI_INT, dest, TAG_sending_merge_output,MPI_COMM_WORLD);
  }
  MPI_Finalize();
  free(output);
  // >>
   return 0;
}
