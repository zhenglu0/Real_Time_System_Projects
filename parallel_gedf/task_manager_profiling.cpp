// Each real time task should be compiled as a separate program and include task_manager.cpp and task.h
// in compilation. The task struct declared in task.h must be defined by the real time task.

#include <sched.h>
#include <unistd.h> 
#include <stdio.h>
#include <math.h>
#include <sstream>
#include <signal.h>
#include <omp.h>
#include <iostream>
#include "task.h"
#include "single_use_barrier.h"
#include "timespec_functions.h"

#include <stdlib.h>

enum rt_gomp_task_manager_error_codes
{ 
	RT_GOMP_TASK_MANAGER_SUCCESS,
	RT_GOMP_TASK_MANAGER_CORE_BIND_ERROR,
	RT_GOMP_TASK_MANAGER_SET_PRIORITY_ERROR,
	RT_GOMP_TASK_MANAGER_INIT_TASK_ERROR,
	RT_GOMP_TASK_MANAGER_RUN_TASK_ERROR,
	RT_GOMP_TASK_MANAGER_BARRIER_ERROR,
	RT_GOMP_TASK_MANAGER_BAD_DEADLINE_ERROR,
	RT_GOMP_TASK_MANAGER_ARG_PARSE_ERROR,
	RT_GOMP_TASK_MANAGER_ARG_COUNT_ERROR
};

int main(int argc, char *argv[])
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
	
	int priority;
	unsigned first_core, last_core, num_iters;
	long period_sec, period_ns, deadline_sec, deadline_ns, relative_release_sec, relative_release_ns;
	if (!(
		std::istringstream(argv[1]) >> first_core &&
		std::istringstream(argv[2]) >> last_core &&
		std::istringstream(argv[3]) >> priority &&
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
	
	// Bind the task to the assigned cores
	//LJ: Tasks are now bound to their assigned cores in clustering_launcher.cpp.
	//Moved so Cilk Plus would utilize cores above 12
	/*
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
		kill(0, SIGTERM);
		return RT_GOMP_TASK_MANAGER_CORE_BIND_ERROR;
	}
	*/
	
	// Set priority to the assigned real time priority
	sched_param sp;
	sp.sched_priority = priority;
	int ret_val = sched_setscheduler(getpid(), SCHED_FIFO, &sp);
	if (ret_val != 0)
	{
		perror("ERROR: Could not set process scheduler/priority");
		kill(0, SIGTERM);
		return RT_GOMP_TASK_MANAGER_SET_PRIORITY_ERROR;
	}
	
	// Set OpenMP settings
	omp_set_dynamic(0);
	omp_set_nested(0);
	//LJ
	omp_set_schedule(omp_sched_dynamic, 1);
	//omp_set_schedule(omp_sched_guided, 1);
	omp_set_num_threads(omp_get_num_procs());

        //set OMP environmental veriable
        char *variable;
        //variable=getenv("OMP_PROC_BIND");
        //printf("\nbind %s\n",variable);
        //variable=getenv("OMP_WAIT_POLICY");
        //printf("policy %s\n",variable);
        //variable=getenv("GOMP_DYNAMIC");
        //printf("dynamic %s\n",variable);
        setenv("OMP_PROC_BIND","true",1);
        setenv("OMP_WAIT_POLICY","ACTIVE",1);
        //setenv("OMP_WAIT_POLICY","PASSIVE",1);
        setenv("GOMP_DYNAMIC","false",1);
        //variable=getenv("OMP_PROC_BIND");
        //printf("\nbind %s\n",variable);
        //variable=getenv("OMP_WAIT_POLICY");
        //printf("policy %s\n",variable);
        //variable=getenv("GOMP_DYNAMIC");
        //printf("dynamic %s\n",variable);

	
	omp_sched_t omp_sched;
	int omp_mod;
	omp_get_schedule(&omp_sched, &omp_mod);
	fprintf(stderr, "OMP sched: %u %u\n", omp_sched, omp_mod);
	
	fprintf(stderr, "Initializing task %s\n", task_name);

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
	
	fprintf(stderr, "Task %s reached barrier\n", task_name);
	
	// Wait at barrier for the other tasks
	ret_val = await_single_use_barrier(barrier_name);
	if (ret_val != 0)
	{
		fprintf(stderr, "ERROR: Barrier error for task %s", task_name);
		kill(0, SIGTERM);
		return RT_GOMP_TASK_MANAGER_BARRIER_ERROR;
	}
	
	// Initialize timing controls
	unsigned deadlines_missed = 0;
	timespec correct_period_start, period_finish, period_runtime; //, actual_period_start
	get_time(&correct_period_start);
	correct_period_start = correct_period_start + relative_release;
	timespec max_period_runtime = { 0, 0 };
	timespec max_overhead = { 0, 0 };

	timespec profiling[102];
	timespec profmax = {0,0};
	int profaccuracy = (int) num_iters/100;
	int profiter = 0;
	
	for (unsigned i = 0; i < num_iters; ++i)
	{
		// Sleep until the start of the period
		sleep_until_ts(correct_period_start);
		//get_time(&actual_period_start);
	
		// Run the task
		ret_val = task.run(task_argc, task_argv);
		get_time(&period_finish);
		if (ret_val != 0)
		{
			fprintf(stderr, "ERROR: Task run failed for task %s", task_name);
			return RT_GOMP_TASK_MANAGER_RUN_TASK_ERROR;
		}
		
		// Check if the task finished before its deadline and record the maximum running time
		//ts_diff(actual_period_start, period_finish, period_runtime);
		ts_diff(correct_period_start, period_finish, period_runtime);
		if (period_runtime > deadline) deadlines_missed += 1;
		if (period_runtime > max_period_runtime) max_period_runtime = period_runtime;
                if (period_runtime > deadline) {
                        timespec thei = { i+1, 0 };
			ts_diff(deadline, period_runtime, max_overhead);
                        max_overhead = max_overhead + thei;
			if (max_overhead.tv_nsec > max_period_runtime.tv_nsec) max_period_runtime = max_overhead;}
		
		// Update the period_start time
		if (period_runtime > period) correct_period_start = period_finish;
		else correct_period_start = correct_period_start + period;

		if (i%profaccuracy == 0){
			profiling[profiter]=profmax;
			profiter++;
			profmax = period_runtime;}
		else{
			if(profmax<period_runtime) profmax=period_runtime;}

	}
	
        sleep_until_ts(correct_period_start);

	// Finalize the task
	if (task.finalize != NULL) 
	{
		ret_val = task.finalize(task_argc, task_argv);
		if (ret_val != 0)
		{
			fprintf(stderr, "WARNING: Task finalization failed for task %s\n", task_name);
		}
	}
	
	std::cerr << "Deadlines missed for task " << task_name << ": " << deadlines_missed << " / " << num_iters << std::endl;
	std::cerr << "Max running time for task " << task_name << ": " << max_period_runtime << " secs" << std::endl;
	std::cout << deadlines_missed << " " << num_iters << " " << omp_get_num_procs() << " " << max_period_runtime;
	//std::cout << deadlines_missed << " " << num_iters << " " << omp_get_num_procs() << " " << maxiteration;

	std::cout<<std::endl;
	for (profiter = 0; profiter<102; profiter++){
		std::cout << profiling[profiter].tv_sec<<" "<<profiling[profiter].tv_nsec<<std::endl;}


	fflush(stdout);
	
	return 0;
}

