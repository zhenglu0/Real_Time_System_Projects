/* Copyright (C) 2005, 2007, 2008, 2009, 2010 Free Software Foundation, Inc.
   Contributed by Richard Henderson <rth@redhat.com>.

   This file is part of the GNU OpenMP Library (libgomp).

   Libgomp is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   Libgomp is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
   FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
   more details.

   Under Section 7 of GPL version 3, you are granted additional
   permissions described in the GCC Runtime Library Exception, version
   3.1, as published by the Free Software Foundation.

   You should have received a copy of the GNU General Public License and
   a copy of the GCC Runtime Library Exception along with this program;
   see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
   <http://www.gnu.org/licenses/>.  */

/* This file contains data types and function declarations that are not
   part of the official OpenMP user interface.  There are declarations
   in here that are part of the GNU OpenMP ABI, in that the compiler is
   required to know about them and use them.

   The convention is that the all caps prefix "GOMP" is used group items
   that are part of the external ABI, and the lower case prefix "gomp"
   is used group items that are completely private to the library.  */

#ifndef LIBGOMP_H 
#define LIBGOMP_H 1

#include "gstdint.h"

#include <pthread.h>
#include <stdbool.h>

//Added by RT_GOMP project
//Needed to set processor affinity masks
#include <sched.h>
//Specify either the FIFO scheduler or a Round Robin scheuler
#define LINUX_RT_SCHED SCHED_FIFO
//This is to avoid repeatedly calling sched_get_priority_min, and is set
//in the function rt_omp_team_Start
//unsigned linux_min_prio;

//Needed for our currently-hacky error reporting
#include <stdio.h>


#include "sem.h"
#include "mutex.h"
#include "bar.h"
#include "ptrlock.h"

/* This structure contains the data to control one work-sharing construct,
   either a LOOP (FOR/DO) or a SECTIONS.  */
//KRAGE 1-18 : added GFS_RT
enum gomp_schedule_type
{
  GFS_RUNTIME,
  GFS_STATIC,
  GFS_DYNAMIC,
  GFS_GUIDED,
  GFS_RT,
  GFS_AUTO
};

//  Forward declaration of struct rt_omp_worker

struct rt_omp_worker;

/*	Added by the RT_GOMP project

		This structure describes a team of real-time threads. Currently these
		are only spawned by the function RT_GOMP_parallel_start.

		A team consists of a set of threads that reside on processors from
		start_proc to end_proc, inclusive. 

		Author: David Ferry */

struct rt_omp_team
{
	/* This is a pointer to an array of pointers to threads, and is used to
		 dispatch work to threads. The worker threads occupy slots
		 from zero to nthreads - 1. */
	pthread_t* worker_threads;

	/* This barrier synchronizes before and FOR loops */
	gomp_barrier_t barrier;

	//Total number of threads in the team- for convinience
	unsigned nthreads;

	// This is the first processor that will have a team thread
	// The originally calling thread will be on this core
	unsigned start_proc;

	// This is the last processor that will have a team thread
	unsigned end_proc;

};	

/* Added by the RT GOMP project

	 This structure holds data used to pass over the pthread_create barrier when
	 making real-time worker threads.

	 Author: David Ferry */

struct rt_omp_worker_create_helper
{
	//In this iteration the only time a team is created or destroyed is at
	//the start or end of a PARALLEL directive, so we pass the function to
	//execute right here
	void (*fn) (void*);
	void* data;

	// A pointer to this worker's team
	struct rt_omp_team* team;

	// Worker index, used so a thread can locate itself in the various team arrays
	unsigned worker_index;

	// The CPU that this thread will execute on
	unsigned cpu;
};

/* Added by the RT GOMP project

	 This structure holds data used to pass over the pthread_create barrier when
	 making real-time worker threads.

	 Author: David Ferry */


/* Added by the RT_GOMP project
		
	 This structure contains all the thread private data for a worker thread.
	 Author: David Ferry */
struct rt_omp_worker
{
	//The thread's current team
	struct rt_omp_team* team;

	//This thread's assigned cpu
	unsigned cpu;
	
	//Worker index, used so a thread can locate itself in various team arrays
	unsigned worker_index;
};

//Thread local data:
extern __thread struct rt_omp_worker rt_omp_tls_data;

/* Function prototypes.  */

/* affinity.c */


/* alloc.c */
extern void *gomp_malloc (size_t) __attribute__((malloc));
extern void *gomp_malloc_cleared (size_t) __attribute__((malloc));
extern void *gomp_realloc (void *, size_t);

/* Avoid conflicting prototypes of alloca() in system headers by using
   GCC's builtin alloca().  */
#define gomp_alloca(x)  __builtin_alloca(x)

/* error.c */

extern void gomp_error (const char *, ...)
  __attribute__((format (printf, 1, 2)));
extern void gomp_fatal (const char *, ...)
  __attribute__((noreturn, format (printf, 1, 2)));



/* iter_ull.c */


/* ordered.c */


/* parallel.c */


/* proc.c (in config/) */


/* task.c */

/* team.c */

/* work.c */

/* Now that we're back to default visibility, include the globals.  */

/* Include omp.h by parts.  */

#endif /* LIBGOMP_H */
