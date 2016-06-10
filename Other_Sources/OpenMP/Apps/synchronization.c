#include <stdio.h>
#include <omp.h>


int main()

{
  int a[] = {2,3,4,6,7,8,9,0}; 
  int i, max = a[0],SIZE = 8;
  
  #pragma omp parallel for num_threads(4)
  for (i = 1; i < SIZE; i++) { 
    if (a[i] > max) { 
#pragma omp critical  
      // compare a[i] and max again because max
      // could have been changed by another thread after
      // the comparison outside the critical section
      if (a[i] > max)
	max = a[i]; 
    } 
  }
  printf("The max number is %d \n",max);
  return 0;
}
