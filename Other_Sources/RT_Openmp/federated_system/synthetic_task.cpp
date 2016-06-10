#include <omp.h>
#include <sstream>
#include <stdio.h>
#include "task.h"
#include "timespec_functions.h"
#include <unistd.h>
#include <math.h>

//LJ
int init(int argc, char *argv[])
{
	//set up the openmp runtime
	//by using parallel-for loop
	int iterations = 10;

	//get num of cores
	cpu_set_t process_mask;
	int err = sched_getaffinity(getpid(), sizeof(process_mask),&process_mask);
	//int err = pthread_getaffinity_np (pthread_self(), sizeof(process_mask),&process_mask);
	if (0 == err)
	{
		int j = 0;
		int available_cores = 0;
		for (j = 0; j < CPU_SETSIZE; j++){
			if (CPU_ISSET(j, &process_mask))available_cores++;
		}
		//std::cerr << "Number of cores: "<<available_cores<<std::endl;
		iterations = pow(2, available_cores);
	}

	//parallel-for loop
	timespec segment_length = {0, 4000000};
	int j;
	#pragma omp parallel for schedule(runtime)
	for (j = 0; j < iterations; ++j)
	{
		busy_work(segment_length);
	}

	return 0;
}

int run(int argc, char *argv[])
{
    if (argc < 1)
    {
        fprintf(stderr, "ERROR: Two few arguments");
	    return -1;
    }
	    
    int num_segments;
    if (!(std::istringstream(argv[1]) >> num_segments))
    {
        fprintf(stderr, "ERROR: Cannot parse input argument");
        return -1;
    }
    
    
    // For each segment
	int i;
	for (i = 0; i < num_segments; ++i)
	{
	    if (argc < 5 + 3*i)
	    {
	        fprintf(stderr, "ERROR: Two few arguments");
		    return -1;
	    }
	    
	    int num_strands;
	    long len_sec, len_ns;
	    if (!(
	        std::istringstream(argv[2 + 3*i]) >> num_strands &&
	        std::istringstream(argv[3 + 3*i]) >> len_sec &&
	        std::istringstream(argv[4 + 3*i]) >> len_ns
	    ))
	    {
	        fprintf(stderr, "ERROR: Cannot parse input argument");
		    return -1;
	    }
	    
		timespec segment_length = { len_sec, len_ns };
		
		// For each strand in parallel
		int j;
		#pragma omp parallel for schedule(runtime)
		for (j = 0; j < num_strands; ++j)
		{
		    // Perform work
			busy_work(segment_length);
		}
		
	}
	
	return 0;
}

task_t task = { init, run, NULL };

