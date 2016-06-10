#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

/* Include the LITMUS^RT API.*/
#include "litmus.h"

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

#define HZ      3430275000      /* second */
#define MHZ     3430275         /* millic second */
#define UHZ     3430.275        /* micro second */

#define MAX	2000           /* max recorded data */

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

struct RECORD {
    unsigned long long dispatch;             /* job release time */
    unsigned long long finish;               /* job deadline */
    timespec relative_deadline;
};

RECORD rt_thread_data[MAX];

/* A real-time thread is very similar to the main function of a single-threaded
 * real-time app. Notice, that init_rt_thread() is called to initialized per-thread
 * data structures of the LITMUS^RT user space libary.
 * The time unit is millisecond*/
void rt_thread(timespec PERIOD, timespec RELATIVE_DEADLINE, int ID)
{
	int do_exit;
	struct rt_task param;
    rt_thread_data[ID].relative_deadline = RELATIVE_DEADLINE;
	/* Set up task parameters */
	init_rt_task_param(&param);
	param.exec_cost = s2ns(RELATIVE_DEADLINE.tv_sec) + RELATIVE_DEADLINE.tv_nsec;
	param.period = s2ns(PERIOD.tv_sec) + PERIOD.tv_nsec;
	param.relative_deadline = s2ns(RELATIVE_DEADLINE.tv_sec) + RELATIVE_DEADLINE.tv_nsec;
	/* What to do in the case of budget overruns? */
	//param.budget_policy = NO_ENFORCEMENT;
	/* The task class parameter is ignored by most plugins. */
	//param.cls = RT_CLASS_SOFT;
	/* The priority parameter is only used by fixed-priority plugins. */
	//param.priority = LITMUS_LOWEST_PRIORITY;
	param.cpu = 1;
	//be_migrate_to(3);
	/* Make presence visible. */
	/* Initialize real-time settings. */
	init_rt_thread();
	/* Set param ID*/
	set_rt_task_param(gettid(), &param);
	/* Transition to real-time mode. */
	task_mode(LITMUS_RT_TASK);

	return;
}

#endif /* RT_GOMP_TASK_H */