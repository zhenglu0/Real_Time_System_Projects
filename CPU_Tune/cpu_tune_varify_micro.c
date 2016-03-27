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
#define HZ      3092259000      /* second */
#define MHZ     3092259         /* millic second */
#define UHZ     3092        /* micro second */

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
} RECORD;

/*
 * Task related parameters, time unit is milli second
 */
int wcet = 0;                   /* worest case execution time */
int period = 0;                 /* period (we assume deadline equals period) */
long long duration = 0;         /* task execution duration */
int priority = 0;               /* task priority */
int count = 0;                  /* number of jobs to execute (duration / period) */
ticks start_time;               /* program start time */
int idx = 0;                    /* job index */
struct RECORD data[MAX];        /* recorded data */

/*
 * print out the results, calcuate deadline miss ratio for this task
 */
static void print_res(void) 
{
	int miss = 0;              /* number of deadline misses */
	int i = 0;

	printf("Release         Start           Finish\n");
	for ( i = 0; i < count; ++i ) {
        printf("%-15lld %-15lld %-15lld\n", start_time/UHZ, data[i].dispatch/UHZ, data[i].finish/UHZ);

        if ( data[i].finish > start_time+period*UHZ )
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
    for ( i = 0; i < 54500; ++i ) {
        temp = sqrt((double)i*i);
        sqrt(temp);
    }  
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

    //struct timeval cur_time1, cur_time2, tdiff;
    //gettimeofday(&cur_time1,NULL);

    data[idx].dispatch = rdtsc();
    for ( i = 0; i < wcet; ++i ) {
        workload_for_1ms();
    }
    data[idx].finish = rdtsc();

    /*
    gettimeofday(&cur_time2,NULL);
    tdiff.tv_sec = cur_time2.tv_sec - cur_time1.tv_sec -1;
    tdiff.tv_usec = cur_time2.tv_usec + (1000000 - cur_time1.tv_usec);
    while(tdiff.tv_usec > 1000000)
    {
      tdiff.tv_sec++;
      tdiff.tv_usec -= 1000000;
      printf("updated tdiff tv_sec:%ld tv_usec:%ld\n",tdiff.tv_sec, tdiff.tv_usec);
    }

    printf("end tdiff tv_sec:%ld tv_usec:%ld\n",tdiff.tv_sec, tdiff.tv_usec);
    */
    ++idx;

}

static void Usage(void)
{
    fprintf(stderr, "Usage: ./period -c wcet -p period -d duration -l priority\n");
    exit(EXIT_FAILURE);
}

/*
 * Set affinity of the task, alwasy pin it to core 0
 */
static void set_sched(void)
{
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(0, &mask);
    if (sched_setaffinity(0, sizeof(cpu_set_t), &mask) < 0) {
        perror("sched_setaffinity");
        exit(EXIT_FAILURE);
    }

    struct sched_param sched;
    sched.sched_priority = priority;
    if (sched_setscheduler(getpid(), SCHED_FIFO, &sched) < 0) {
        perror("sched_setscheduler");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    sigset_t allsigs;
    int cur_val = 0;

    while ((cur_val = getopt(argc, argv, "c:p:d:l:")) != -1) {
        switch (cur_val) {
            case 'c':
                wcet = atoi(optarg);        
                if ( wcet <= 0 ) {
                    printf("wcet must be greater than zero\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 'p':
                period = atoi(optarg);
                if ( period <= 0 ) {
                    printf("period must be greater than zero\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 'd':
                duration = atol(optarg);
                if ( duration <= 0 ) {
                    printf("duration must be greater than zero\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 'l':
                priority = atoi(optarg);
                if ( priority <= 0 ) {
                    printf("priority must be greater than zero\n");
                    exit(EXIT_FAILURE);
                }
                break;
            default:
                printf("Error Input!\n");
                Usage();
                exit(EXIT_FAILURE);
                break;
        }
    }

    if ( wcet == 0 || period == 0 || duration == 0 || priority == 0) {
        Usage();
        exit(1);
    }

    count = (duration - 1) / period + 1;  /* number of jobs to release */
    printf("wcet: %d, period: %d, duration: %lld, count: %d\n", wcet, period, duration, count);

	set_sched();

    struct sigaction sa;

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = work;

    if (sigaction(SIGRTMIN, &sa, NULL) < 0) {
        perror("sigaction error");
        exit(EXIT_FAILURE);
    }

    /* the timer */
    struct itimerspec timerspec;
    timerspec.it_interval.tv_sec = period / 1000;
    timerspec.it_interval.tv_nsec = (period % 1000) * 1000000;

    /* the start time */
    struct timespec now;
    if(clock_gettime(CLOCK_REALTIME, &now) < 0) {
        perror("clock_gettime");
        exit(EXIT_FAILURE);
    }

    // Start one second from now.
    timerspec.it_value.tv_sec = now.tv_sec + 1;
    timerspec.it_value.tv_nsec = now.tv_nsec;
	start_time = rdtsc();
	start_time += HZ;

    struct sigevent timer_event;
    timer_t timer;
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

