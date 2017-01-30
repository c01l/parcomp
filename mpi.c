#include <stdio.h>
#include <mpi.h>
#include <limits.h>
#include "loader.h"
#include "corank.h"

struct local_data {
  int rank;
  int num_of_processes;
  int *part_A;
  int *part_B;
  int part_A_size;
  int part_B_size;
  MPI_Win *win_A;
  MPI_Win *win_B;
  int input_array1_length;
  int input_array2_length;
};

int mpilogging = 1;

int getBlocksize(int totalArrayLength, int numberOfProcesses) {
  return totalArrayLength / numberOfProcesses;
}
int getBlocksizeOfOutput(struct local_data localData) {
  // so far ignore the rank and assume array length is divisible by numberOfProcesses
  int size = (localData.input_array1_length + localData.input_array2_length) / localData.num_of_processes;
  return size;
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

int getValFromA(int index, struct local_data localData) {
  return getVal(index, localData.input_array1_length, *localData.win_A, localData.num_of_processes, localData.rank, localData.part_A);
}

int getValFromB(int index, struct local_data localData) {
  return getVal(index, localData.input_array2_length, *localData.win_B, localData.num_of_processes, localData.rank, localData.part_B);
}


struct pair_of_coranks corank_mpi(int i, struct local_data localData) {
  int sizeA = localData.input_array1_length, sizeB = localData.input_array2_length;
  int mpiSize = localData.num_of_processes, rank = localData.rank;
  int *A = localData.part_A, *B = localData.part_B;
  MPI_Win win1 = *localData.win_A, win2 = *localData.win_B;

  struct pair_of_coranks coranks;
  int j = min(i, sizeA);
  int k = (i - j);
  int j_low = max (0, (i- sizeA));
  int k_low = 0;
  int counter = 0;

  while(counter < sizeA + sizeB) {
    // if A[j-1] > B[k]
    if(j > 0 && k < sizeB && getValFromA(j-1, localData) >  getValFromB(k, localData)) {
      k_low = k;
      int diff = j - j_low;
      int delta = diff/2 + (diff&1);
      j = j - delta;
      k = k + delta;
    // if B[k-1] >= B[j]

    } else if (k>0 && j<sizeA && getValFromB(k-1, localData) >= getValFromA(j, localData)){
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


void localMerge(int* output,  struct pair_of_coranks corank1, struct pair_of_coranks corank2,  struct local_data localData) {
  int i;
  // get the entries between coranks
  struct merge_sample merge_payload;
  merge_payload.size1 = corank2.corank_A - corank1.corank_A;
  merge_payload.size2 = corank2.corank_B - corank1.corank_B;
  merge_payload.array1 = malloc(sizeof(int) * merge_payload.size1);
  merge_payload.array2 = malloc(sizeof(int) * merge_payload.size2);
  for(i = 0; i < merge_payload.size1; i++) {
    merge_payload.array1[i] = getValFromA(i + corank1.corank_A, localData); 
  }
  for(i = 0; i < merge_payload.size2; i++) {
    merge_payload.array2[i] = getValFromB(i + corank1.corank_B, localData); 
  }
  // printf("#%i merge payload (size1,size2) (%i,%i)\n", localData.rank, merge_payload.size1, merge_payload.size2);
  //<< MERGE
  int totalLen = getBlocksizeOfOutput(localData);
  if(merge_payload.size1 == 0){
    for(i = 0; i< totalLen; i++) output[i] = merge_payload.array2[i];
  } else if (merge_payload.size2 == 0) {
    for(i = 0; i< totalLen; i++) output[i] = merge_payload.array1[i];
  } else {
    merge(&merge_payload, output, 0, totalLen);
  }
  freesample(&merge_payload);
  // MERGE >>
}


int main(int argc, char** argv)
{
  int TAG_sending_input_part1 = 100;
  int TAG_sending_input_part2 = 101;
  int TAG_sending_merge_output = 102;
  struct merge_sample sample; 
  struct local_data localData;
  
  // set up MPI variables
  MPI_Init(&argc,&argv);
  int mpiRank, mpiSize, i;
  MPI_Comm_size(MPI_COMM_WORLD, &mpiSize); //total number of processes -> mpiSize
  MPI_Comm_rank(MPI_COMM_WORLD, &mpiRank); //process number -> mpiRank
  MPI_Win winA, winB;
  int *A, *B;

  // put values in local_data struct
  localData.rank = mpiRank;
  localData.num_of_processes = mpiSize;

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
    int len = sizeA + sizeB;
    A = malloc(sizeof(INPUTTYPE) * len1);
    B = malloc(sizeof(INPUTTYPE) * len2);

    // put values to local_data struct:
    localData.part_A = A;
    localData.part_B = B;
    localData.part_A_size = getBlocksize(sample.size1, mpiSize);
    localData.part_B_size = getBlocksize(sample.size2, mpiSize);
    localData.input_array1_length = sample.size1;
    localData.input_array2_length = sample.size2;


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
    for(i = 1; i < mpiSize; i++){
      MPI_Send(array1parts[i], len1, MPI_INT, i, TAG_sending_input_part1,MPI_COMM_WORLD);
      MPI_Send(array2parts[i], len2, MPI_INT, i, TAG_sending_input_part2,MPI_COMM_WORLD);
    }
    
    for(i = 0; i < len1; i++) A[i] = array1parts[0][i];
    for(i = 0; i < len2; i++) B[i] = array2parts[0][i];
  } else {
    // GET input from RANK 0
    MPI_Status mpiStatus;
    MPI_Recv(A, len1, MPI_INT, 0, TAG_sending_input_part1, MPI_COMM_WORLD,&mpiStatus);
    MPI_Recv(B, len2, MPI_INT, 0, TAG_sending_input_part2, MPI_COMM_WORLD,&mpiStatus);
  }
  
  // now that every process has its parts, start to benchmark
  double start = MPI_Wtime();

  // enable access to A and B for other processes
  MPI_Win_create(A, len1, sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD, &winA);
  MPI_Win_create(B, len2, sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD, &winB);
  localData.win_A = &winA;
  localData.win_B = &winB;
  

  // coranking
  struct pair_of_coranks corank1, corank2;
  int index1 = mpiRank*len/mpiSize;
  int index2 = (mpiRank + 1)*len/mpiSize;
  corank1 = corank_mpi(index1, localData);
  corank2 = corank_mpi(index2, localData);
  if(corank2.corank_A - corank1.corank_A < 0 || corank2.corank_B - corank1.corank_B < 0){
    printf("ERROR: wrong coranks for rank %i!\n", localData.rank);
  }

  // Merge
  int *output = malloc(sizeof(int) * getBlocksizeOfOutput(localData));
  localMerge(output, corank1, corank2, localData);

  // free windows and stop time
  MPI_Win_free(&winA);
  MPI_Win_free(&winB);
  double end = MPI_Wtime();


  // << Put everything together in Rank 0:
  if(mpiRank == 0){
    int *result = malloc(sizeof(int) * len);
    for(i = 0; i< getBlocksizeOfOutput(localData); i++) result[i] = output[i]; // write rank 0's items to result
    int offset = getBlocksize(len,mpiSize); 
    for(i = 1; i < mpiSize; i++) {
      MPI_Status status;
      MPI_Recv(result + (i*offset), offset, MPI_INT, i, TAG_sending_merge_output, MPI_COMM_WORLD, &status);
      int elemenets;
      MPI_Get_count(&status, MPI_INT, &elemenets);
    }

    //check result
    testIfSorted(result, len);
    free(result);
    free(output);
  } else {
    // the rest sends their merge-outputs to 0
    int dest = 0;
    int numberOfElements = getBlocksize(len, mpiSize);
    MPI_Send(output, numberOfElements, MPI_INT, dest, TAG_sending_merge_output,MPI_COMM_WORLD);
  }
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize();
  // >>
    printf("Time: %fs\n", end - start);
   return 0;
}
