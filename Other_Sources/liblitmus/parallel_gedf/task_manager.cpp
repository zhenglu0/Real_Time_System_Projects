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
#include "single_use_barrier.h"
#include "timespec_functions.h"

#include "litmus.h"


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
    int priority;
	unsigned first_core, last_core, num_iters;
	long period_sec, period_ns, deadline_sec, deadline_ns, 
		 relative_release_sec, relative_release_ns;
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
		fprintf(stderr, "ERROR: Cannot parse input argument for task %s",
                task_name);
		kill(0, SIGTERM);
		return RT_GOMP_TASK_MANAGER_ARG_PARSE_ERROR;
	}

	char *barrier_name = argv[11];
	int task_argc = argc - (num_req_args-1);
	char **task_argv = &argv[num_req_args-1];
		
	timespec period = { period_sec, period_ns };
	timespec deadline = { deadline_sec, deadline_ns };
	timespec relative_release = { relative_release_sec, relative_release_ns };
	
	//std::cout << "period = " << period << std::endl;
	//std::cout << "deadline = " << deadline << std::endl;
	
	// Check if the task has a run function
	if (task.run == NULL)
	{
        fprintf(stderr, "ERROR: Task does not have a run function %s",
                task_name);
		kill(0, SIGTERM);
		return RT_GOMP_TASK_MANAGER_RUN_TASK_ERROR;
	}
	
    unsigned num_cores;
    num_cores = last_core - first_core + 1;
    int ret_val;
    //int ret_val, NUM_THREADS = 1;//omp_get_num_procs();
	/* Set OpenMP settings */
    omp_set_dynamic(0);
    omp_set_nested(0);
    omp_set_schedule(omp_sched_static,1);
    omp_set_num_threads(num_cores);


    fprintf(stderr, "Initializing task %s\n", task_name);

	// Initialize the task
	if (task.init != NULL)
	{
		ret_val = task.init(task_argc, task_argv);
		if (ret_val != 0)
		{
			fprintf(stderr, "ERROR: Task initialization failed for task %s",
                    task_name);
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
    timespec period_finish, period_runtime; //, actual_period_start
    //get_time(&correct_period_start);
    //correct_period_start = relative_release;
    timespec correct_period_start = {0, priority*1000000};
    timespec max_period_runtime = { 0, 0 };
    timespec max_overhead = { 0, 0 };
    sleep_until_ts(correct_period_start);

	init_litmus();

    #pragma omp parallel for
	for (unsigned i = 0; i < num_cores; i++) 
	{
		//timespec decrease = {0, i*100000 };
		//timespec decrease = {0, 0};
        //timespec relative_deadline_diff;
		//ts_diff (deadline, decrease, relative_deadline_diff);
		rt_thread(period, deadline,priority, i);
		wait_for_ts_release();
	}
	std::cerr << "release task " << priority << std::endl;
	fflush(stderr);
	int samestart = int(relative_release*1000 / period);
    std::cerr << "Period " << period << " hyper " << relative_release << " sleep " << samestart << std::endl;
	fflush(stdout);
	/*
	while (samestart != 0)
	{
		#pragma omp parallel for
		for (unsigned i = 0; i < num_cores; i++)
		{
			sleep_next_period();
		}
		samestart--;
	}
	*/

    get_time(&correct_period_start);
	//for (unsigned iter = 0; iter < num_iters; ++iter) 
	//for (unsigned i = 0; i < num_iters; ++i) 
	//Only for running experiments!!!!!!!!!!!!!!!!!!!!!!!!!!
	for (int i = 0; i < samestart; ++i) 
	{
		//sleep_until_ts(correct_period_start);
    
        // Run the task
        //get_time(&correct_period_start);
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
        //std::cerr<<"start "<<correct_period_start<<" finish "<<period_finish;
        //std::cerr<<" runtime "<<period_runtime<<" deadline "<<deadline<<std::endl;
        if (period_runtime > deadline) deadlines_missed += 1;
        //if (period_runtime > max_period_runtime) max_period_runtime = period_runtime;
        if (period_runtime > deadline) {
                        timespec thei = { i+1, 0 };
            ts_diff(deadline, period_runtime, max_overhead);
                        max_overhead = max_overhead + thei;
            if (max_overhead > max_period_runtime) max_period_runtime = max_overhead;}
        
        // Update the period_start time
        //correct_period_start = correct_period_start + period;
        if (period_runtime > period) correct_period_start = period_finish;
        else correct_period_start = correct_period_start + period;

	    #pragma omp parallel for 
		for (unsigned i = 0; i < num_cores; i++) 
		{	
			sleep_next_period();
		}
        //timespec now;
        //get_time(&now);
        //std::cerr<<"finish "<<now<<"start"<<correct_period_start<<std::endl;
        //fflush(stdout);

	}	

    // Finalize the task
	if (task.finalize != NULL) 
	{
		ret_val = task.finalize(task_argc, task_argv);
		if (ret_val != 0)
		{
			fprintf(stderr, "WARNING: Task finalization failed for task %s\n",
                    task_name);
		}
	}
    	
    std::cerr << "Deadlines missed for task " << task_name << ": " << 
				deadlines_missed << " / " << num_iters << std::endl;
    std::cerr << "Max running time for task " << task_name << ": " << 
				max_period_runtime << " secs" << std::endl;
    //SG: removed omp_get_num_procs() replaced with "( last_core - first_core + 1 )"
    std::cout << deadlines_missed << " " << num_iters << " " << 
				( last_core - first_core + 1 ) << " " << max_period_runtime;
    //std::cout << deadlines_missed << " " << num_iters << " " << 
	// ( last_core - first_core + 1 ) << " " << maxiteration;
    fflush(stdout);
	
	return 0;
}

