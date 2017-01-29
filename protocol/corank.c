  while(counter < (m + n)) {
    if( j>0  &&  k<n  &&  A[j-1]>B[k] ) {
      k_low = k;
      int diff = j-j_low;
      int delta = diff/2 + (diff&1);
      j = j - delta;
      k = k + delta;
    } else if ( k>0  &&  j<m  &&  B[k-1]>=A[j]  ) {
      j_low = j; 
      int diff = k-k_low;
      int delta = diff/2 + (diff&1);
      k = k - delta;
      j = j + delta;
    } else {
      coranks.corank_A = j;
      coranks.corank_B = k;
    }
    counter++;
  }
