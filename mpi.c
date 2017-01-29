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

  while(counter < sizeA + sizeB) {
    // if A[j-1] > B[k]
    if(j > 0 && k < sizeB && getVal(j-1,sizeA, win1, mpiSize, rank, A) >  getVal(k, sizeB, win2, mpiSize, rank, B)) {
      k_low = k;
      int diff = j - j_low;
      int delta = diff/2 + (diff&1);
      j = j - delta;
      k = k + delta;
    // if B[k-1] >= B[j]
    } else if (k>0 && j<sizeA && getVal(k-1, sizeB, win2, mpiSize, rank, B) >= getVal(j, sizeA, win1, mpiSize, rank, A)){
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
  if(mpiRank == 0) {
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
  }

    //split up array for rank 0
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
    // GET input from RANK 0
    MPI_Status mpiStatus;
    MPI_Recv(A, len1, MPI_INT, 0, TAG_sending_input_part1, MPI_COMM_WORLD,&mpiStatus);
    MPI_Recv(B, len2, MPI_INT, 0, TAG_sending_input_part2, MPI_COMM_WORLD,&mpiStatus);
  }

  double start = MPI_Wtime();
   

  // enable access to A and B for other processes
   MPI_Win_create(A, len1, sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD, &winA);
   MPI_Win_create(B, len2, sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD, &winB);
  

  // << CORANK
  struct pair_of_coranks corank1, corank2;
  int index1 = mpiRank*len/mpiSize;
  int index2 = (mpiRank + 1)*len/mpiSize;
  corank1 = corank_mpi(index1, mpiRank,sizeA,sizeB , winA, winB, mpiSize, A, B);
  corank2 = corank_mpi(index2, mpiRank,sizeA,sizeB , winA, winB, mpiSize, A, B);
  // CORANK>>
  printf("#%i successfully calculated corank1 (%i, %i) and corank2(%i, %i)\n", mpiRank, corank1.corank_A, corank1.corank_B, corank2.corank_A, corank2.corank_B);

  //<< LOCAL MERGE
  // get the entries between coranks
  struct merge_sample merge_payload;
  merge_payload.size1 = corank2.corank_A - corank1.corank_A;
  merge_payload.size2 = corank2.corank_B - corank1.corank_B;
  merge_payload.array1 = malloc(sizeof(int) * merge_payload.size1);
  merge_payload.array2 = malloc(sizeof(int) * merge_payload.size2);
  for(i = 0; i < merge_payload.size1; i++) {
    merge_payload.array1[i] = getVal(i + corank1.corank_A, sizeA, winA, mpiSize, mpiRank, A); 
  }
  for(i = 0; i < merge_payload.size2; i++) {
    merge_payload.array2[i] = getVal(i + corank1.corank_B, sizeB, winB, mpiSize, mpiRank, B); 
  }
  printf("#%i merge payload (%i,%i)\n", mpiRank, merge_payload.size1, merge_payload.size2);
  MPI_Win_free(&winA);
  MPI_Win_free(&winB);
  printf("#%i successfully got elements to merge\n", mpiRank);

  //<< MERGE
  int totalLen = merge_payload.size1 + merge_payload.size2;
  int *output = malloc(sizeof(int) * totalLen);
  if(merge_payload.size1 == 0){
    for(i = 0; i< totalLen; i++) output[i] = merge_payload.array2[i];
  } else if (merge_payload.size2 == 0) {
    for(i = 0; i< totalLen; i++) output[i] = merge_payload.array1[i];
  } else {
    merge(&merge_payload, output, 0, totalLen);
  }
  freesample(&merge_payload);
  printf("#%i successfully merged\n", mpiRank);
  echoArray(output, 5);
  // MERGE >>

  double end = MPI_Wtime();


  // << Put everything together in Rank 0:
  if(mpiRank == 0){
    int *result = malloc(sizeof(int) * len);
    for(i = 0; i< totalLen; i++) result[i] = output[i]; // write rank 0's items to result

    // receive all parts async:
    int offset = getBlocksize(len,mpiSize); 
    // MPI_Request *recv_request = malloc(sizeof(MPI_Request) * (mpiSize - 1));
    for(i = 1; i < mpiSize; i++) {
      MPI_Status *status;
      MPI_Recv(result + (i*offset), offset, MPI_INT, i, TAG_sending_merge_output, MPI_COMM_WORLD, status);
      int elemenets;
      MPI_Get_count(status, MPI_INT, &elemenets);
      printf("#%i received %i elements from %i\n", mpiRank, elemenets, (*status).MPI_SOURCE);
      // MPI_Irecv(result + (i*offset), offset, MPI_INT, i, TAG_sending_merge_output, MPI_COMM_WORLD, recv_request + (i - 1));
    }
    // wait until all parts have been received
    // MPI_Status mpiStatus;
    //for(i=1; i<mpiSize; i++) MPI_Wait(recv_request + (i - 1), &mpiStatus);
    printf("#%i successfully received all parts\n", mpiRank);
    //free(recv_request);

    //check result
    testIfSorted(result, len);
    free(result);
    free(output);
  } else {
    // the rest sends their merge-outputs to 0
    int dest = 0;
    int numberOfElements = getBlocksize(len, mpiSize);
    printf("#%i is about to send %i elements to %i\n", mpiRank, numberOfElements, dest);
    echoArray(output, 5);
    MPI_Send(output, numberOfElements, MPI_INT, dest, TAG_sending_merge_output,MPI_COMM_WORLD);
    printf("#%i successfully sent merged part to rank %i\n", mpiRank, dest);
  }
  printf("#%i about to finalize\n", mpiRank);
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize();
  printf("#%isuccessfully finalized\n", mpiRank);
  // >>
    printf("Time: %fs\n", end - start);
   return 0;
}
