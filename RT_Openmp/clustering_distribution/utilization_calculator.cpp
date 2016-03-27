// Usage: program_name first_core last_core bucket_width_sec bucket_width_ns num_repetitions arg1 arg2 ...

#include <stdio.h>
#include <iostream>
#include <sstream>
#include <map>
#include <omp.h>
#include "task.h"
#include "timespec_functions.h"
#include "histogram.h"

enum rt_gomp_utilization_calculator_error_codes
{
	RT_GOMP_UTILIZATION_CALCULATOR_SUCCESS,
	RT_GOMP_UTILIZATION_CALCULATOR_INIT_ERROR,
	RT_GOMP_UTILIZATION_CALCULATOR_RUN_ERROR,
	RT_GOMP_UTILIZATION_CALCULATOR_CORE_BIND_ERROR,
	RT_GOMP_UTILIZATION_CALCULATOR_SET_PRIORITY_ERROR,
	RT_GOMP_UTILIZATION_CALCULATOR_ARG_PARSE_ERROR,
	RT_GOMP_UTILIZATION_CALCULATOR_ARG_COUNT_ERROR
};

int main(int argc, char *argv[])
{
	// Verify number of arguments
	const int num_req_args = 6;
	if (argc < num_req_args)
	{
		fprintf(stderr, "ERROR: Too few arguments");
		return RT_GOMP_UTILIZATION_CALCULATOR_ARG_COUNT_ERROR;
	}
	
	// Parse arguments
	unsigned first_core, last_core, num_repetitions;
	long bucket_width_sec, bucket_width_ns;
	if (!(
		std::istringstream(argv[1]) >> first_core &&
		std::istringstream(argv[2]) >> last_core &&
		std::istringstream(argv[3]) >> bucket_width_sec &&
		std::istringstream(argv[4]) >> bucket_width_ns &&
		std::istringstream(argv[5]) >> num_repetitions
	))
	{
		fprintf(stderr, "ERROR: Cannot parse input argument");
		return RT_GOMP_UTILIZATION_CALCULATOR_ARG_PARSE_ERROR;
	}
	
	// Get task_argc and task_argv from argc and argv by
	// leaving out the profiling arguments. Copy the program
	// name to argv[num_req_args-1] and use that position as task_argv[0]
	// so that the caller of utilization_calculator does not
	// need to repeat the program name.
	argv[num_req_args-1] = argv[0];
	int task_argc = argc - (num_req_args-1);
	char **task_argv = &argv[num_req_args-1];
	
	// Check if the task has a run function
	if (task.run == NULL)
	{
		fprintf(stderr, "ERROR: Task does not have a run function");
		return RT_GOMP_UTILIZATION_CALCULATOR_RUN_ERROR;
	}
	
	// Bind to cores
	cpu_set_t mask;
	CPU_ZERO(&mask);
	for (unsigned i = first_core; i <= last_core; ++i)
	{
		CPU_SET(i, &mask);
	}
	
	int ret_val = sched_setaffinity(getpid(), sizeof(mask), &mask);
	if (ret_val != 0)
	{
		perror("ERROR: Could not set CPU affinity");
		return RT_GOMP_UTILIZATION_CALCULATOR_CORE_BIND_ERROR;
	}
	
	// Set priority to a real time priority
	sched_param sp;
	sp.sched_priority = 97;
	ret_val = sched_setscheduler(getpid(), SCHED_FIFO, &sp);
	if (ret_val != 0)
	{
		perror("ERROR: Could not set process scheduler/priority");
		return RT_GOMP_UTILIZATION_CALCULATOR_SET_PRIORITY_ERROR;
	}
	
	// Set OpenMP settings
	omp_set_dynamic(0);
	omp_set_nested(0);
	omp_set_schedule(omp_sched_dynamic, 1);
	omp_set_num_threads(omp_get_num_procs());
	
	omp_sched_t omp_sched;
	int omp_mod;
	omp_get_schedule(&omp_sched, &omp_mod);
	fprintf(stderr, "OMP sched: %u %u\n", omp_sched, omp_mod);
	
	// Initialize a histogram to record the profiling results
	const timespec start_with = { 0, 0 };
	const timespec bucket_width = { bucket_width_sec, bucket_width_ns };
	Histogram<timespec> histogram(start_with, bucket_width);

	// Initialize task
	if (task.init != NULL)
	{
		ret_val = task.init(task_argc, task_argv);
		if (ret_val != 0)
		{
			fprintf(stderr, "ERROR: Task initialization failed");
			return RT_GOMP_UTILIZATION_CALCULATOR_INIT_ERROR;
		}
	}
	
	// Repeatedly profile runs of the task
	timespec start, finish, runtime;
	for (unsigned i = 0; i < num_repetitions; ++i)
	{
		get_time(&start);
		
		ret_val = task.run(task_argc, task_argv);
		
		get_time(&finish);
		
		if (ret_val != 0)
		{
			fprintf(stderr, "ERROR: Task run failed");
			return RT_GOMP_UTILIZATION_CALCULATOR_RUN_ERROR;
		}
		
		ts_diff(start, finish, runtime);
		histogram.add_observation(runtime);
	}
	
	// Finalize the task
	if (task.finalize != NULL)
	{
		ret_val = task.finalize(task_argc, task_argv);
		if (ret_val != 0)
		{
			fprintf(stderr, "WARNING: Task finalization failed\n");
		}
	}
	
	// Print out the histogram of profiling results
	std::cout << histogram << std::endl;
	
	return 0;
}

