#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <sched.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <omp.h>

/* Include the LITMUS^RT API.*/
#include "litmus.h"

/* Catch errors.
 */
#define CALL( exp ) do { \
    int ret; \
    ret = exp;	\
    if (ret != 0)				\
        fprintf(stderr, "%s failed: %m\n", #exp);	\
    else						\
        fprintf(stderr, "%s ok.\n", #exp);		\
} while (0)


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

/*
 * Global data structure to record the data
 * The output is per-task.
 * To minimize interference, you should dump it to /dev/shm (shared memory)
 */
#define MAX	20000           /* max recorded data */

typedef unsigned long long ticks;
typedef struct RECORD {
    ticks dispatch;             /* job release time */
    ticks finish;               /* job deadline */
    int deadline;
    cpu_set_t mask;
    int cpu_id;
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
struct RECORD data[MAX];             /* recorded data */
struct RECORD rt_thread_data[MAX];
/*
 * print out the results, calcuate deadline miss ratio for this task
 */
static void print_res(void)
{
	int miss = 0;              /* number of deadline misses */
	int i = 0;

	printf("Release         Start           Finish\n");
	for ( i = 0; i < count; ++i ) {
        printf("%-15lld %-15lld %-15lld\n", start_time/MHZ, data[i].dispatch/MHZ, data[i].finish/MHZ);

        if ( data[i].finish > start_time+period*MHZ )
            miss++;

        start_time += period*MHZ;
    }

	printf("\nTotal %d, Deadline Miss %d, Ratio %f\n", count, miss, (float)miss/(float)count);
}

/*
 * 1ms workload on a specific machine
 * You need to tune this value or change workload
 */
static inline void workload_for_1ms(void)
{
    long long i;
    double temp;
    for ( i = 0; i < 190000; ++i ) {
        temp = sqrt((double)i*i);
	sqrt(temp);
    }
}

int job(int rt_thread_id)
{
    //rt_thread_data[rt_thread_id].dispatch = rdtsc();
    int i = 0,j = 0;
    cpu_set_t mask_;
    pthread_t thread;
    /* Do real-time calculation. */
    for ( i = 0; i < wcet; ++i ) {
        workload_for_1ms();
    }
    /* Don't exit. */
    rt_thread_data[rt_thread_id].finish = rdtsc();
    thread = pthread_self();
    //sched_getaffinity(pid_t pid, size_t cpusetsize, mask_);
    pthread_getaffinity_np(thread, sizeof(cpu_set_t),&mask_);

    printf("pthread id = %u !!!!!!!", (unsigned)thread);

    rt_thread_data[rt_thread_id].mask = mask_;
    for (j = 0; j < 4; j++) {
        if (CPU_ISSET(j, &mask_)) {
            printf("j = %d ",j);
            rt_thread_data[rt_thread_id].cpu_id = j;
        }
    }
    printf("\n");
    return 0;
}

/* A real-time thread is very similar to the main function of a single-threaded                    * real-time app. Notice, that init_rt_thread() is called to initialized per-thread                * data structures of the LITMUS^RT user space libary.                                             * The time unit is millisecond
*/
void rt_thread(int wcet, int period, int relative_deadline, int rt_thread_id)
{
    struct rt_task param;
    rt_thread_data[rt_thread_id].deadline = relative_deadline;
    /* Set up task parameters */
    init_rt_task_param(&param);
    param.exec_cost = ms2ns(wcet);
    param.period = ms2ns(period);
    param.relative_deadline = ms2ns(relative_deadline);
    printf("exec_cost = %d, period = %d, relative_deadline = %d \n",
	   wcet, period, relative_deadline);
    /* What to do in the case of budget overruns? */
    param.budget_policy = NO_ENFORCEMENT;
    /* The task class parameter is ignored by most plugins. */
    param.cls = RT_CLASS_SOFT;
    /* The priority parameter is only used by fixed-priority plugins. */
    param.priority = LITMUS_LOWEST_PRIORITY;

    /*
     * 1) Initialize real-time settings.
     */
    CALL(init_rt_thread());
    CALL(set_rt_task_param(gettid(), &param));
    /*
     * 2) Transition to real-time mode.
     */
    CALL(task_mode(LITMUS_RT_TASK));
    /* The task is now executing as a real-time task if the call didn't fail.
     */
    /*
     * 4) Transition to background mode.
     */
    CALL(task_mode(BACKGROUND_TASK));
    /* The task is now executing as a real-time task if the call didn't fail.
     * 3) Invoke real-time jobs.
     */
    /* Invoke job. */
    //rt_thread_data[rt_thread_id].dispatch = rdtsc();
    //job();
    //rt_thread_data[rt_thread_id].finish = rdtsc();
}

/*
 * each job's work
 * record start time, finish time
 */
static void work(int sig, siginfo_t *extra, void *cruft)
{
    int i;
    /* We have reached the count. Print res and quit */
    if (idx >= count) {
        sleep(2);              /* sleep for 10 sec, wait for other task to finish */
        print_res();
        exit(1);
    }
    printf("started \n");
    data[idx].dispatch = rdtsc();
    #pragma omp parallel for
    for (i = 0; i < omp_get_num_procs()*3; ++i) {
        rt_thread(wcet, period, relative_deadline-(int)(i/omp_get_num_procs()), i);
    }

    for (i = 0; i < omp_get_num_procs()*3; ++i) {
        rt_thread_data[i].dispatch = data[idx].dispatch;
    }


    #pragma omp parallel for
    for (i = 0; i < omp_get_num_procs()*3; ++i) {
        job(i);
    }

    printf("finished \n\n");
    data[idx].finish = rdtsc();
    ++idx;

    printf("start         Finish           RelativeDeadline       Execution time    Mask \n");
    for ( i = 0; i < omp_get_num_procs()*3; ++i ) {
      printf("%-15lld %-15lld %d     %-15lld    %d \n",
	     rt_thread_data[i].dispatch/MHZ, rt_thread_data[i].finish/MHZ,
	     rt_thread_data[i].deadline,
	     rt_thread_data[i].finish/MHZ-rt_thread_data[i].dispatch/MHZ,
	     rt_thread_data[i].cpu_id);
    }
}

int main(int argc, char *argv[]) {
    sigset_t allsigs;
    struct sigaction sa;
    struct timespec now;
    struct sigevent timer_event;
    struct itimerspec timerspec;
    timer_t timer;

    /* Set OpenMP settings */
    omp_set_dynamic(0);
    omp_set_nested(0);
    omp_set_schedule(omp_sched_static,1);
    omp_set_num_threads(omp_get_num_procs()*3);

    //count = (duration - 1) / period + 1;  /* number of jobs to release */
    printf("wcet: %d, period: %d, duration: %lld, count: %d\n", wcet, period, duration, count);

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = work;

    if (sigaction(SIGRTMIN, &sa, NULL) < 0) {
        perror("sigaction error");
        exit(EXIT_FAILURE);
    }

    timerspec.it_interval.tv_sec = period / 1000;
    timerspec.it_interval.tv_nsec = (period % 1000) * 1000000;
    /* the start time */
    if(clock_gettime(CLOCK_REALTIME, &now) < 0) {
        perror("clock_gettime");
        exit(EXIT_FAILURE);
    }
    // Start one second from now.
    timerspec.it_value.tv_sec = now.tv_sec + 1;
    timerspec.it_value.tv_nsec = now.tv_nsec;
    start_time = rdtsc();
    start_time += HZ;

    timer_event.sigev_notify = SIGEV_SIGNAL;
    timer_event.sigev_signo = SIGRTMIN;
    timer_event.sigev_value.sival_ptr = (void *)&timer;

    if (timer_create(CLOCK_REALTIME, &timer_event, &timer) < 0) {
        perror("timer_create");
        exit(EXIT_FAILURE);
    }

    if (timer_settime(timer, TIMER_ABSTIME, &timerspec, NULL) < 0) {
        perror("timer_settime");
        exit(EXIT_FAILURE);
    }

    sigemptyset(&allsigs);
    while(1) {
        sigsuspend(&allsigs);
    }

    exit(EXIT_SUCCESS);
}

