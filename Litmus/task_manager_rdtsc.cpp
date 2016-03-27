/* based_task.c -- A basic multi-threaded real-time task skeleton.
 *
 * This (by itself useless) task demos how to setup a multi-threaded LITMUS^RT
 * real-time task. Familiarity with the single threaded example (base_task.c)
 * is assumed.
 *
 * Currently, liblitmus still lacks automated support for real-time
 * tasks, but internaly it is thread-safe, and thus can be used together
 * with pthreads.
 */
#include "task.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <omp.h>
/* Include gettid() */
#include <sys/types.h>

/* Include threading support. */
#include <pthread.h>

/* Include the LITMUS^RT API.*/
#include "litmus.h"

/* The information passed to each thread. Could be anything. */
struct thread_context {
        int id;
        int EXEC_COST;
        int PERIOD; 
        int RELATIVE_DEADLINE;
};

/*
 * Read the rdtsc value
 */
#if defined(__i386__)
static __inline__ unsigned long long rdtsc(void)
{
  unsigned long long int x;
     __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
     return x;
}
#elif defined(__x86_64__)
static __inline__ unsigned long long rdtsc(void)
{
  unsigned hi, lo;
  __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
  return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}
#endif

/*
 * Useful CPU related paramters
 * These are based on specific CPU parameters.
 * You need to change them according to your machine
 * cat /proc/cpuinfo    for more info
 */
#define HZ      3430275000      /* second */
#define MHZ     3430275         /* millic second */
#define UHZ     3430.275            /* micro second */

#define MAX	20000           /* max recorded data */

typedef unsigned long long ticks;
typedef struct RECORD {
    ticks dispatch;             /* job release time */
    ticks finish;               /* job deadline */
    int deadline;
} RECORD;

/*
 * Task related parameters, time unit is milli second
 */
int wcet = 1000;                    /* worest case execution time */
int period = 2000;                  /* period (we assume deadline equals period) */ 
int relative_deadline = 2000;       /* relative_deadline (we assume deadline equals period) */
long long duration = 10000;         /* task execution duration */
int count = 1;                       /* number of jobs to execute (duration / period) */
ticks start_time;                    /* program start time */
int idx = 0;                         /* job index */
struct RECORD rt_thread_data[MAX];
int argc;							/* For main() */		            					
char** argv;						/* For main() */

/* The real-time thread program. Doesn't have to be the same for
 * all threads. Here, we only have one that will invoke job().
 */
void rt_thread(int wcet, int period, int relative_deadline, int rt_thread_id);
/* Declare the periodically invoked job.
 * Returns 1 -> task should exit.
 *         0 -> task should continue.
 */

/* Catch errors.
 */
#define CALL( exp ) do { \
		int ret; \
		ret = exp; \
		if (ret != 0) \
			fprintf(stderr, "%s failed: %m\n", #exp);\
		else \
			fprintf(stderr, "%s ok.\n", #exp); \
	} while (0)


/* Basic setup is the same as in the single-threaded example. However,
 * we do some thread initiliazation first before invoking the job.
 */
int main(int argc_, char** argv_)
{
	// Intinalize parameters
	argc = argc_;
	argv = argv_;
	size_t num_iters = 2;
	// Initialize the task
	if (task.init)
		task.init(argc, argv);

    int i, NUM_THREADS = 11;
    //NUM_THREADS = omp_get_num_procs()*10;
	/* Set OpenMP settings */
    //omp_set_dynamic(0);
    //omp_set_nested(0);
    //omp_set_schedule(omp_sched_static,1);
    omp_set_num_threads(omp_get_num_procs()*3);
	/*****
	 * Initialize LITMUS^RT.
	 * Task parameters will be specified per thread.
	 */
	init_litmus();
	/*****
	 * Launch threads.
	 */
	printf("started \n");

	for (size_t iter = 0; iter < num_iters; ++iter) {
	    #pragma omp parallel for
		for (i = 0; i < NUM_THREADS; i++) {
			rt_thread(wcet, period, relative_deadline - i*100, i);
		}
		/*****
		 * Clean up, maybe print results and stats, and exit.
		 */
		printf("start         Finish           RelativeDeadline\n");
	    for ( i = 0; i < NUM_THREADS; ++i ) {
	        printf("%-15lld %-15lld %d\n", 
		           rt_thread_data[i].dispatch/MHZ, rt_thread_data[i].finish/MHZ, 
		           rt_thread_data[i].deadline);
	    }
	}

	// Finalize the task
	if (task.finalize)
		task.finalize(argc, argv);

	return 0;
}

/* A real-time thread is very similar to the main function of a single-threaded
 * real-time app. Notice, that init_rt_thread() is called to initialized per-thread
 * data structures of the LITMUS^RT user space libary.
 * The time unit is millisecond*/
void rt_thread(int EXEC_COST, int PERIOD, int RELATIVE_DEADLINE, int ID)
{
	int do_exit;
	struct rt_task param;
    rt_thread_data[ID].dispatch = rdtsc();
    rt_thread_data[ID].deadline = RELATIVE_DEADLINE;
	/* Set up task parameters */
	init_rt_task_param(&param);
	param.exec_cost = ms2ns(EXEC_COST);
	param.period = ms2ns(PERIOD);
	param.relative_deadline = ms2ns(RELATIVE_DEADLINE);
    printf("exec_cost = %d, period = %d, relative_deadline = %d \n", 
	   		EXEC_COST, PERIOD, RELATIVE_DEADLINE);
	/* What to do in the case of budget overruns? */
	param.budget_policy = NO_ENFORCEMENT;
	/* The task class parameter is ignored by most plugins. */
	param.cls = RT_CLASS_SOFT;
	/* The priority parameter is only used by fixed-priority plugins. */
	param.priority = LITMUS_LOWEST_PRIORITY;

	/* Make presence visible. */
	printf("RT Thread %d active.\n", ID);
	/*****
	 * 1) Initialize real-time settings.
	 */
	CALL( init_rt_thread() );

	/* To specify a partition, do
	 *
	 * param.cpu = CPU;
	 * be_migrate_to(CPU);
	 *
	 * where CPU ranges from 0 to "Number of CPUs" - 1 before calling
	 * set_rt_task_param().
	 */
	CALL( set_rt_task_param(gettid(), &param) );

	/*****
	 * 2) Transition to real-time mode.
	 */
	CALL( task_mode(LITMUS_RT_TASK) );

	/* The task is now executing as a real-time task if the call didn't fail.
	 * 3) Invoke real-time jobs.
	 */
	do {
		/* Wait until the next job is released. */
		sleep_next_period();
		rt_thread_data[ID].dispatch = rdtsc();
		// Run the task
    	if (task.init)
			do_exit = task.run(argc, argv);
	} while (!do_exit);

    rt_thread_data[ID].finish = rdtsc();
	/*****
	 * 4) Transition to background mode.
	 */
	CALL( task_mode(BACKGROUND_TASK) );

	return;
}

