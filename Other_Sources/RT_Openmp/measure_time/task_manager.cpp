// Each real time task should be compiled as a separate program and include task_manager.cpp and task.h
// in compilation. The task struct declared in task.h must be defined by the real time task.

#include <unistd.h>
#include <iostream> 
#include "task.h"
#include "timespec_functions.h"

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
#define UHZ     3430.275            /* micro second */

int main(int argc, char *argv[])
{

	// Initialize the task
	if (task.init)
		task.init(argc, argv);

    timespec actual_period_start, period_finish, period_runtime;
	
	get_time(&actual_period_start);
    // Run the task
    //if (task.init)
		//task.run(argc, argv);
    get_time(&period_finish);

    ts_diff(actual_period_start, period_finish, period_runtime);

    std::cout << "Period_runtime time for task " << period_runtime * 1000000000 << " secs" << std::endl;

	// Finalize the task
	if (task.finalize)
		task.finalize(argc, argv);


	unsigned long long a = rdtsc();
	unsigned long long b = rdtsc();

	std::cout << "time = " << (double)(b-a)/(HZ/1000000000) << std::endl;


	return 0;
}

