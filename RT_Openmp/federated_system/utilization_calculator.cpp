// Usage: program_name first_core last_core bucket_width_sec bucket_width_ns num_repetitions arg1 arg2 ...

#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <fstream>
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
	
	// Initialize a histogram to record the profiling results
	const timespec start_with = { 0, 0 };
	timespec bucket_width = { bucket_width_sec, bucket_width_ns };
	timespec max = { 0, 0 };
	timespec mean = { 0, 0 };

	if (bucket_width == mean) {
	// Repeatedly profile runs of the task
	timespec start_test, finish_test, runtime_test;
	timespec diff_test;
        timespec max_test = {0,0};
        timespec min_test = {100000000,0};
	int numtest = num_repetitions/100;
	if (numtest < 5) numtest = 5;
	for (int i = 0; i < numtest; ++i)
	{
		get_time(&start_test);
		
		ret_val = task.run(task_argc, task_argv);
		
		get_time(&finish_test);
		
		if (ret_val != 0)
		{
			fprintf(stderr, "ERROR: Task run failed");
			return RT_GOMP_UTILIZATION_CALCULATOR_RUN_ERROR;
		}
		
		ts_diff(start_test, finish_test, runtime_test);
		if (max_test < runtime_test) max_test = runtime_test;
		if (min_test > runtime_test) min_test = runtime_test;
	}
	ts_diff(min_test, max_test, diff_test);
	bucket_width = diff_test/(((double)numtest)*4);
	std::cout << bucket_width << std::endl;
	}

	Histogram<timespec> histogram(start_with, bucket_width);
	timespec allexe[num_repetitions];

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
		allexe[i] = runtime;
		if (max < runtime) max = runtime;
		mean = mean + runtime;
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
	
	std::ofstream output;
	std::stringstream ss;
	ss << (last_core-first_core+1);
	std::string ss0 = "results/execution_time_omp_"+ss.str()+".dat";
	//output.open("execution_time_"+string(last_core-first_core+1)+".txt");
	output.open(ss0.c_str());
	for (unsigned i = 0; i < num_repetitions; ++i)
	{
		output << allexe[i] << std::endl;
	}
	output.close();

	// Print out the histogram of profiling results
	std::cout << histogram << std::endl;
	mean =  mean / num_repetitions;
	timespec calculated = bucket_width; // Temporarily using bucket_width to pass in the calculated running time
	std::cout << calculated << " | " << max << " | " << mean << std::endl;
	
	return 0;
}

