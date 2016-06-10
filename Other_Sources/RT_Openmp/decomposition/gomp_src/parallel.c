#include "team.c"
#include "../gomp_src/libgomp.h"
#include <limits.h>

/*	Added by the RT_OMP project

		Creates a team that executes function fn with data, and has a worker thread
		on every core from start_proc to end_proc, inclusive. E.g. if start_proc is
		0 and end_proc is 11, then 12 total threads will comprise the team.
		*/
void 
RT_OMP_parallel_start (void (*fn) (void*), void* data, unsigned start_proc, unsigned end_proc, const char* sem_name)
{

	//Only the creating thread will actually execute rt_omp_team_start
	//rt_omp_new_team simply sets up the data structures necessary
  //rt_omp_team_start will create threads and send them on their way
	rt_omp_team_start (fn, data, start_proc, end_proc, rt_omp_new_team (start_proc, end_proc), sem_name);

}

/*	Added by the RT_OMP project
  	
		To terminate a parallel team we must send a signal to each thread telling
		it to quit. 
		*/
void 
RT_OMP_parallel_end (void)
{

}

