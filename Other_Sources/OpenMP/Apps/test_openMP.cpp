// Set OpenMP settings
omp_set_dynamic(0);
omp_set_nested(0);
omp_set_schedule(omp_sched_dynamic, 1);
omp_set_num_threads(omp_get_num_procs());
omp_sched_t omp_sched;
int omp_mod;
omp_get_schedule(&omp_sched, &omp_mod);
fprintf(stderr, "OMP sched: %u %u\n", omp_sched, omp_mod);
fprintf(stderr, "Initializing task\n");
//然后你就正常调用init，run和final就应该可以。我也把platform里面调用时候包含测deadline miss等相关的代码粘在底下了，但是里面有不少你需要改了才能用，所以仅供参考：
// Initialize the task
if (task.init != NULL)
{
    ret_val = task.init(task_argc, task_argv);
    if (ret_val != 0)
    {
        fprintf(stderr, "ERROR: Task initialization failed for task %s", task_name);
        kill(0, SIGTERM);
        return 1;
    }
}
fprintf(stderr, "Task finishes initialization\n");
// Initialize timing controls
unsigned deadlines_missed = 0;
timespec correct_period_start, period_finish, period_runtime; //, actual_period_start
get_time(&correct_period_start);
correct_period_start = correct_period_start + relative_release;
timespec max_period_runtime = { 0, 0 };
timespec max_overhead = { 0, 0 };

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
    if (period_runtime > deadline)
        deadlines_missed += 1;
	//if (period_runtime > max_period_runtime) max_period_runtime = period_runtime;
        if (period_runtime > deadline)
        {
            timespec thei = { i+1, 0 };
	    ts_diff(deadline, period_runtime, max_overhead);
            max_overhead = max_overhead + thei;
	    if (max_overhead > max_period_runtime) max_period_runtime = max_overhead;
        }

    // Update the period_start time
    if (period_runtime > period) 
      correct_period_start = period_finish;
    else 
      correct_period_start = correct_period_start + period;
}

sleep_until_ts(correct_period_start);

// Finalize the task
if (task.finalize != NULL)
{
    ret_val = task.finalize(task_argc, task_argv);
    if (ret_val != 0)
    {
        fprintf(stderr, "WARNING: Task finalization failed for task\n");
    }
}

std::cerr << "Deadlines missed for task " << task_name << ": " << deadlines_missed << " / " << num_iters << std::endl;
std::cerr << "Max running time for task " << task_name << ": " << max_period_runtime << " secs" << std::endl;
std::cout << deadlines_missed << " " << num_iters << " " << omp_get_num_procs() << " " << max_period_runtime;
//std::cout << deadlines_missed << " " << num_iters << " " << omp_get_num_procs() << " " << maxiteration;
fflush(stdout);
