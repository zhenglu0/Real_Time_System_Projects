/* This file handles the maintainence of threads in response to team
   creation and termination.  */

#include "libgomp.h"
#include <stdlib.h>
#include <string.h>
#include "semaphore.c"
#include "time_utils.h"

/* This is the libgomp per-thread data structure.  */
__thread struct rt_omp_worker rt_omp_tls_data;

/*  This function is the entry point for all worker threads.

		The worker threads will barrier wait until they are given work to do. We
		expect an idling thread to be lowest priority in the system and to be
		asleep most of the time.
*/

void*
rt_omp_worker_entry (void* xdata)
{
	void (*fn) (void*);
	void* data;

	//Initialize local variables and team struct
	rt_omp_worker_create_helper* helper = (rt_omp_worker_create_helper*)xdata;
	rt_omp_worker* local = &rt_omp_tls_data;

	//Set up the thread-local data
	local->team = helper->team;
	local->worker_index = helper->worker_index;
	local->cpu = helper->cpu;
		
	fn = helper->fn;
	data = helper->data;	

	//Wait for all other threads to finish being constructed before proceeding
	//The creator thread will not pass this barrier until after intial inter-
	//process synchronization is complete
	gomp_barrier_wait(&(local->team->barrier));

	//Execute the desired function
	fn(data);

	//When fn returns we assume we have completed the PARALLEL section, and
	//clean up.
	
	//Destroy helper struct
	free(helper);

	pthread_exit(NULL);
}

/* Create a new rt_omp_team data structure.

	 We assume that processor numbers are positive and that 
	 end_proc is greater than start_proc. 
	 */

struct rt_omp_team*
rt_omp_new_team (unsigned start_proc, unsigned end_proc)
{
	if(start_proc > end_proc){
		fprintf(stderr,"ERROR: End processor is greater than start processor in rt_omp_new_team!\n");
		abort();
	}

	struct rt_omp_team *team;
	size_t size;

	//Total number of threads in the system
	unsigned nthreads = end_proc - start_proc + 1;

	size = sizeof(rt_omp_team) + (nthreads)*(sizeof(team->worker_threads[0]));
	team = (rt_omp_team*)gomp_malloc (size);

	team->nthreads = nthreads;
	team->start_proc = start_proc;
	team->end_proc = end_proc;
	gomp_barrier_init(&team->barrier,nthreads);

	//Pointer arithmetic, we want worker_threads to be contiguous with its team 
	team->worker_threads = (pthread_t*)&team + sizeof(rt_omp_team);

	return team;
}

/*	This function takes a team data structure and actually instantiates the
		worker threads. It pins the executing thread to start_proc, and every other
		thread to start_proc + 1 to end_proc.

		This also does the initial inter-process synchronization through the
		semaphore sem_name
		*/

void
rt_omp_team_start (void (*fn) (void*), void* data, unsigned start_proc, unsigned end_proc, rt_omp_team* team, const char* sem_name)
{

	/* Create each thread in the system, bind it to its processor, and set its
		 initial prioirty to to be the system minimum. 

		 This process is highly nonportable off Linux systems due to affinity
		 setting. */

	int ret_val;
	cpu_set_t mask;

	//The less or equals is so that a thread is created on end_proc
	for ( unsigned i = start_proc; i <= end_proc; i++ ){
		//We want our data structures to be zero indexed		
		unsigned index = i - start_proc;
	
		//Package thread initialization data to send across pthread_create
		rt_omp_worker_create_helper* helper = (rt_omp_worker_create_helper*)gomp_malloc(sizeof(rt_omp_worker_create_helper));
		helper->team = team;
		helper->worker_index = index;
		helper->cpu = i;
		helper->fn = fn;
		helper->data = data;	
	
		//The first thread in every team will be the thread that is currently
		//executing- that is, the one that is setting up the team. A new thread is
		//not needed.
		
		if( i == start_proc ){
			team->worker_threads[index] = pthread_self();
			//These assignments only happen within the constructing thread
			rt_omp_tls_data.team = team;
			rt_omp_tls_data.worker_index = index;
			rt_omp_tls_data.cpu = i;
		} else {
			//Create threads
			ret_val = pthread_create(&(team->worker_threads[index]),NULL,rt_omp_worker_entry,(void*)helper);
			if (ret_val != 0){
				fprintf(stderr,"ERROR: Worker thread creation failed in rt_omp_new_team, reason: %s\n",strerror(ret_val));
				exit(-1);
			}
		
		}

		//Bind each thread to a specific processor
		CPU_ZERO( &mask );
		CPU_SET( i, &mask);
		ret_val = pthread_setaffinity_np(team->worker_threads[index], sizeof(mask), &mask);	
		if (ret_val != 0){
			fprintf(stderr,"WARNING: Could not set CPU affinity in rt_omp_new_team, eason: %s\nWorker thread ID: %lu, Processor: %d\n", strerror(ret_val), team->worker_threads[index],i);
		}

		sched_param sp;
		sp.sched_priority = sched_get_priority_max(LINUX_RT_SCHED);

		//Set initial thread priority to max, set scheduling policy
		ret_val = pthread_setschedparam(team->worker_threads[index],LINUX_RT_SCHED,&sp);
		if (ret_val != 0){
			fprintf(stderr,"WARNING: Could not set thread scheduler in rt_omp_new_team, reason: %s\nWorker Thread ID: %lu, Priority: %d", strerror(ret_val), team->worker_threads[index],sp.sched_priority);
		}		

	
	} //end for

	//We must perform the inital synchronization with all other teams on the
	//system
	//Get a handle to the semaphore object

// TODO: reinstate semaphore waiting when we go back to multiple-tasks 
//	volatile semaphore* initSync = getSemaphore(sem_name);
//	decrement(initSync);

	/* We need to wait on this semaphore to become true, but we can't just
		 busy wait or else we'll be busy waiting at the maximum realtime priority.
		 The result is that two threads might vary in their start times by as much
		 as the sleep_time value. */	
// TODO: resinstate semaphore waiting when we go back to multiple tasks
//	timespec sleep_time;
//	sleep_time.tv_sec = 0;
//	sleep_time.tv_nsec = 500000; //half millisecond
//	while (initSync->value > 0){
//		nanosleep(&sleep_time,NULL);	
//	}

	//All workers at this point are waiting on the team barrier, when the
	//constructing thread passes this barrier it means that the initial
	//inter-process initial synchronization is complete, so passing this
	//barrier releases all workers into executing function fn
	gomp_barrier_wait(&(team->barrier));

	//All workers are now running the function fn
	//Finally, the creating thread must also execute the desired code
	fn(data);

	//Join with all created worker threads so we know we're done
	for(unsigned i = 1; i < end_proc - start_proc; i++){
		pthread_join(team->worker_threads[i],NULL);
	} 

}


/* 	This function is passed to worker threads as a way to clean up. The function
		destroys thread-local objects, frees memory, and calls a pthreads
		termination function for all worker threads.

		At team destruction time, this function is passed to each worker thread to
		be executed.
		*/

void
rt_omp_thread_end(void)
{
	//TODO: Implement
}

/* 	Called to clean up and terminate a team of threads.
		*/

void
rt_omp_team_end(rt_omp_team* team)
{
	//TODO: Implement
}



