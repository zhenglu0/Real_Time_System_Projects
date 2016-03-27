// A simple matrix-vector multiplication task that uses OpenMP
// Call the functions with the following arguments in argv: executable_name num_rows num_cols

#include <omp.h>
#include <cmath>
#include <iostream>
#include "task.h"
/*
 * 1ms workload on a specific machine
 * You need to tune this value or change workload
 */

static inline void workload_for_1ms(void)
{
	long long i;
  	double temp;
  	for ( i = 0; i < 54450; ++i ) {
      	temp = sqrt((double)i*i);
      	sqrt(temp);
  	}     
}

int job()
{
  	int i = 0, wcet = 100;
	/* Do real-time calculation. */
	for ( i = 0; i < wcet; ++i ) {
	  workload_for_1ms();
	}
	return 0; 
}

int init(int argc, char *argv[])
{	
	std::cout << "initalization finished " << std::endl;	
	return 0;
}

int run(int argc, char *argv[])
{	
	return job();
}

int finalize(int argc, char *argv[])
{
	std::cout << "finalize finished " << std::endl;
	return 0;
}

task_t task = { init, run, finalize };
