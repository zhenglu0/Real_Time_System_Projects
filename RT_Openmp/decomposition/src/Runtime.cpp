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



#include "Runtime.h"

Runtime::Runtime(){
	//schedule = std::vector< std::vector< Runnable > > ( RTOMP_NUM_CPUS , std::vector< Runnable > ( 0, Runnable(NULL,NULL) ));
}


Runtime::~Runtime(){}

void Runtime::init(unsigned first_proc, unsigned last_proc, char* sem_name, Task& task){
  void* arg_ptr = (void*) &task;
  RT_OMP_parallel_start(execute, arg_ptr, first_proc, last_proc, sem_name); 
}

void Runtime::execute(void* in_arg){

  //A pointer to a task object to run
  Task* task = (Task*) in_arg;

  timespec period = task->period;
  unsigned period_iters = task->iterations;
  const char* name = task->name.c_str();

  //Priority setting structures
  pthread_t self_id = pthread_self();
  int min_prio = sched_get_priority_min(LINUX_RT_SCHED);
  int max_prio = sched_get_priority_max(LINUX_RT_SCHED);

  //This is the per-thread TLS data from RT_GOMP
  unsigned absolute_cpu = rt_omp_tls_data.cpu;
  //unsigned start_proc = rt_omp_tls_data.team->start_proc;
  //unsigned relative_cpu = absolute_cpu - start_proc;

  //Timespecs needed during execution
  timespec start_time;
  timespec period_start;
  timespec period_dl;
  timespec period_end;
  timespec curr_time;

  //Release Guard Values
  timespec seg_release;
  //double int_part;
  //double frac_part;
  //int release_sec;
  //long int release_nsec;

  //Bookkeeping values
  //unsigned periodic_deadline_miss = 0;
  //unsigned seg_iters = 0;

  get_time(start_time);
  period_dl = start_time;

  //Period iterations
  for (unsigned pIter = 0; pIter < period_iters; pIter++) {

    //The start of the current period is the deadline of the last period. We
    //initialize period_dl for pIter=0 above. 
    period_start = period_dl;

    //Add one period to the deadline every period iteration
    //Adds period_dl and ts_period together, stores result in period_dl
    ts_add(period_dl, period, period_dl);

    for( std::vector< Segment >::iterator s_it = task->segments.begin();
         s_it != task->segments.end(); s_it++){

      //Release guard: wait until our relative release time before executing
      //absolute release time = relative release + current period start     
      ts_add( s_it->release, period_start, seg_release);

      //Sleep until the time seg_release
      sleep_until_future_ts(seg_release);

      //At this point we are free to set priorities and do actual work. We get
      //this data below.
      unsigned seg_prio = s_it->priority;
      std::vector< Strand > curr_work = s_it->strands[absolute_cpu];

      //Only do work if there is work to do
      if (curr_work.size() > 0){

        //Each thread sets it's own priority
        //Note that in linux 99 is the highest priority, while 1 is the lowest
        int ret_val = pthread_setschedprio(self_id, seg_prio);
        if( ret_val != 0 ){
          fprintf(stderr, "WARNING: Could not set thread priority in experiment.cpp, reason: %s\n", strerror(ret_val));
        }


        for( std::vector< Strand >::iterator w_it = curr_work.begin();
             w_it != curr_work.end(); w_it++){
          w_it->run();
				}

        //We revert our priority to high
        ret_val = pthread_setschedprio(self_id, max_prio);
        if( ret_val != 0 ){
          fprintf(stderr,"WARNING: Could not set thread priority in experiment.cpp, reason: %s\n", strerror(ret_val));
        }

			} //End strand-execute loop

      //If we're done with all our computation, we want to check deadlines
      //before we go through the end-of-segment barrier
      if (s_it == --(task->segments.end())) { //this is ugly, sorry
        get_time(period_end);

        if (period_end > period_dl) {
          __sync_add_and_fetch(&(task->period_misses[pIter]), 1);
        }
      }

     //Block on barrier so every thread finishes this segment before going on
      gomp_barrier_wait(&(rt_omp_tls_data.team->barrier));

	  } //End segment iterations

   //Now we wait until the next period to progress to next iteration. If we
    //are late then we progress immediately
    get_time(curr_time);
    //We only wait if the current time is less than the total period deadline
    if (curr_time < period_dl) {
      sleep_until_future_ts( period_dl );
    } else { //Otherwise, we have missed our periodic deadline and do not wait

    }

 } //End periodic iterations

  //Make sure we all finish before anyone prints out results
  gomp_barrier_wait(&(rt_omp_tls_data.team->barrier));

}//End Runtime execute function



