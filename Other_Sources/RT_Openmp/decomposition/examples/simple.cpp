/* Copyright (C) 2012 Washington University in St. Louis - All Rights Reserved
 *
 * This file is part of the RT_OpenMP (Real-Time OpenMP) system.
 *
 * The OpenMP name is a registered trademark of the OpenMP Architecture Review
 * Board (ARB). The RT_OpenMP project aims to adapt the OpenMP API for use in 
 * real-time systems, but the RT_OpenMP project is not affiliated with or 
 * endorsed by the OpenMP ARB. 
 *
 * THIS SOFTWARE IS FREE OF CHARGE AND IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
 * MERCHANTABILITY OR FITNESS FOR ANY PARTICULAR PURPOSE. THE AUTHORS MAKE NO
 * CLAIM THAT THE SOFTWARE WILL PROVIDE ADEQUATE REAL-TIME OPERATION FOR YOUR
 * PARTICULAR APPLICATION. IN NO EVENT SHALL WASHINGTON UNIVERSITY OR THE 
 * AUTHORS OF THIS SOFTWARE BE LIABLE FOR ANY DAMAGES ARISING FROM THE USE OF 
 * THIS PRODUCT.
 *
 * You have the right to use, modify, and distribute this source code subject
 * to the restrictions in the LICENSE file. Some files included in this 
 * distribution are subject to the GPL v3 license, so programs produced using 
 * RT_OpenMP must be licensed under the GPL v3. See the LICENSE file for more 
 * details.
 */


/* A simple test of the RT_OMP system. This program creates a team of threads
 * on processors start_cpu to end_cpu and executes a short function for a 
 * desired number of iterations. Some validation output is given at the end.
*/

#include "Constants.h"
#include "Task.h"
#include "Runtime.h"
#include "time_utils.h"
#include "libgomp.h"

#include <cstdlib>
#include <cstdio>
#include <time.h>

// Specify the range of processors to execute on. Includes endpoints. CPUs are
// zero-indexed (i.e. a machine with 4 cores has CPUs 0-3).
unsigned start_cpu = 12;
unsigned end_cpu = 23;

unsigned values[RTOMP_NUM_CPUS];

// We want a function that does some small amount of constant work. We use the
// optimize attribute to ensure that our increments aren't optimized out.
void __attribute__ ((optimize(O0))) small_work(void* args){
  values[rt_omp_tls_data.cpu]++;
  // Uncomment the following to get a message for every strand execution-
  // be careful, this can produce a LOT of output
  //printf("CPU %d processing strand\n", rt_omp_tls_data.cpu);
}

int main(int argc, char* argv[]){

  // Normal C/C++ command line argument handling

  if(argc < 4){
    printf("Usage: (period seconds) (period nanoseconds) (periodic iterations)\n");
		exit(-1);
	}
	
  int      seconds    = atoi(argv[1]);
  long int nanosecs   = atoi(argv[2]);
  unsigned iterations = atoi(argv[3]);

  /*The timespec struct is the standard way of specifying time and time
  *  intervals in RT_OMP. It has the following definition:
  *
  *  struct timespec{
  *    time_t   tv_sec;  //Seconds value, we use an int for time_t
  *    long int tv_nsec; //Nanoseconds value
  *  };
  */
	timespec period;
  period.tv_sec = seconds;
  period.tv_nsec = nanosecs;

  /*Create an empty task object. The constructor has the following signature:
  *  Arg1 = Numerical ID of task
  *  Arg2 = Alphabetic ID of task, or NULL if none is desired
  *  Arg3 = The length of the period in timespec format
  *  Arg4 = The desired number of periodic iterations for this task
  */
  Task task ( 0, NULL, period, iterations);

  // An alternate way to initialize timespecs
  timespec seg_length = {seconds,nanosecs};
  timespec seg_release = {0,0};

  // When setting priorities, remember that in Linux the
  // highest priority is 99, and the lowest priority is 1
	task.addSegment( seg_length, seg_release, 10); // Analog to OMP FOR

  for(unsigned i = start_cpu; i <= end_cpu; i++)
    task.addStrandToLastSeg( small_work, NULL, i); // Adds work to be done to
                                                   // the above segment.

  // Now that we have a task constructed and populated, we may run it. First
  // we construct the Runtime object, and then call init on it to instantiate
  // the runtime threads and execute the task. We also time the total execution
  // of the runtime object for gross validation.
  
  timespec start;
  timespec end;

  get_time(start); // get_time is in time_utils.h

  Runtime rt;
  rt.init(start_cpu, end_cpu, NULL, task);

  get_time(end);

  // We can check the values array for correctness
  for(unsigned i = 0; i < RTOMP_NUM_CPUS; i++){
    printf("values array slot %d contains %d\n",i,values[i]);
	}

  // We can also check the total runtime
  timespec total;
  ts_diff(end, start, total); // ts_diff is in time_utils.h

  printf("\nThe runtime execution (including construction and initialization) took:\n");
  printf("Seconds:     %d\nNanoseconds: %d\n", (int)total.tv_sec, total.tv_nsec);

  return 0;
}

