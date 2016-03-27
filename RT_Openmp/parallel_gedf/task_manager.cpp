#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <unistd.h>
#include <omp.h>
#include <math.h>
#include <signal.h>
#include <sstream>
#include <iostream>
#include <sys/types.h>
#include <pthread.h>
#include "task.h"
#include "task_manager.h"
#include "timespec_functions.h"

int main(int argc, char** argv)
{
	// Process command line arguments
	const char *task_name = argv[0];
	const int num_req_args = 13;
	if (argc < num_req_args)
	{
		fprintf(stderr, "ERROR: Too few arguments for task %s", task_name);
		kill(0, SIGTERM);
		return RT_GOMP_TASK_MANAGER_ARG_COUNT_ERROR;
	}
	unsigned first_core, last_core, num_iters;
	long period_sec, period_ns, deadline_sec, deadline_ns, 
		 relative_release_sec, relative_release_ns;
	if (!(
		//std::istringstream(argv[1]) >> first_core &&
		//std::istringstream(argv[2]) >> last_core &&
		std::istringstream(argv[4]) >> period_sec &&
		std::istringstream(argv[5]) >> period_ns &&
		std::istringstream(argv[6]) >> deadline_sec &&
		std::istringstream(argv[7]) >> deadline_ns &&
		std::istringstream(argv[8]) >> relative_release_sec &&
		std::istringstream(argv[9]) >> relative_release_ns &&
		std::istringstream(argv[10]) >> num_iters
	))
	{
		fprintf(stderr, "ERROR: Cannot parse input argument for task %s", task_name);
		kill(0, SIGTERM);
		return RT_GOMP_TASK_MANAGER_ARG_PARSE_ERROR;
	}

	char *barrier_name = argv[11];
	int task_argc = argc - (num_req_args-1);
	char **task_argv = &argv[num_req_args-1];
		
	timespec period = { period_sec, period_ns };
	timespec deadline = { deadline_sec, deadline_ns };
	timespec relative_release = { relative_release_sec, relative_release_ns };
	
	// Check if the task has a run function
	if (task.run == NULL)
	{
		fprintf(stderr, "ERROR: Task does not have a run function %s", task_name);
		kill(0, SIGTERM);
		return RT_GOMP_TASK_MANAGER_RUN_TASK_ERROR;
	}
	
    int ret_val, NUM_THREADS = 16;//omp_get_num_procs();
	/* Set OpenMP settings */
    omp_set_dynamic(0);
    omp_set_nested(0);
    omp_set_schedule(omp_sched_static,1);
    omp_set_num_threads(NUM_THREADS);

	// Initialize the task
	if (task.init != NULL)
	{
		ret_val = task.init(task_argc, task_argv);
		if (ret_val != 0)
		{
			fprintf(stderr, "ERROR: Task initialization failed for task %s", task_name);
			kill(0, SIGTERM);
			return RT_GOMP_TASK_MANAGER_INIT_TASK_ERROR;
		}
	}

 	// Initialize timing controls
	unsigned deadlines_missed = 0;
	timespec correct_period_start, actual_period_start[NUM_THREADS], 
			 period_finish[NUM_THREADS], period_runtime[NUM_THREADS];
	get_time(&correct_period_start);
	correct_period_start = correct_period_start + relative_release;
	timespec max_period_runtime = { 0, 0 };
	init_litmus();

    #pragma omp parallel for
	for (int i = 0; i < NUM_THREADS; i++) 
	{
		timespec decrease = {0, i*100000 };
		timespec relative_deadline_diff;
		ts_diff (deadline, decrease, relative_deadline_diff);
		rt_thread(period, relative_deadline_diff, i);
	}

	for (size_t iter = 0; iter < num_iters; ++iter) 
	{
		//sleep_until_ts(correct_period_start);
		#pragma omp parallel for
		for (int i = 0; i < NUM_THREADS; i++) 
		{	
			sleep(2);
			get_time(&actual_period_start[i]);
			rt_thread_data[i].dispatch = rdtsc();
			ret_val = task.run(task_argc, task_argv);
			rt_thread_data[i].finish = rdtsc();
			get_time(&period_finish[i]);
			if (ret_val != 0)
				fprintf(stderr, "ERROR: Task run failed for task %s", task_name);
		}

		// Check if the task finished before its deadline and record the maximum running time
		for (int i = 0; i < NUM_THREADS; i++) 
		{
			ts_diff(actual_period_start[i], period_finish[i], period_runtime[i]);
			if (period_runtime[i] > deadline) 
				deadlines_missed += 1;
			if (period_runtime[i] > max_period_runtime) 
				max_period_runtime = period_runtime[i];
			std::cout << "period_runtime[" << i << "] = " << period_runtime[i] << std::endl;
		}
		// Update the period_start time
		correct_period_start = correct_period_start + period;
		
		printf("start         Finish           RelativeDeadline\n");
	    for ( int i = 0; i < NUM_THREADS; ++i ) 
	    {
	        printf("%-15lld %-15lld    ", 
		           rt_thread_data[i].dispatch/MHZ, rt_thread_data[i].finish/MHZ);
	    	std::cout << rt_thread_data[i].relative_deadline << std::endl;
	    }

	    #pragma omp parallel for
		for (int i = 0; i < NUM_THREADS; i++) 
		{	
			sleep_next_period();
		}
	}
	
	// Finalize the task
	if (task.finalize != NULL) 
	{
		ret_val = task.finalize(task_argc, task_argv);
		if (ret_val != 0)
		{
			fprintf(stderr, "WARNING: Task finalization failed for task %s\n", task_name);
		}
	}
	
	std::cerr << "Deadlines missed for task " << task_name << ": " 
	<< deadlines_missed << " / " << num_iters*NUM_THREADS << std::endl;
	std::cerr << "Max running time for task " << task_name << ": " 
	<< max_period_runtime << " secs" << std::endl;
	
	return 0;
}

