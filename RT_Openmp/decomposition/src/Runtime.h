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


#ifndef _RTOMP_RUNTIME_H_
#define _RTOMP_RUNTIME_H_

#include <vector>
#include <cstdio>
#include <string.h>
#include "Task.h"
#include "time_utils.h"
#include "../gomp_src/libgomp.h"

//Forward declarations from parallel.c and bar.c
void RT_OMP_parallel_start (void (*)(void*), void*, unsigned, unsigned, const char*);

class Runtime {
public:

  Runtime();
  ~Runtime();

  //TODO - eventually we want to look at the task class and construct a static
  //schedule from that, rather than calling straight into it at runtime
  //void makeSchedule(const Task& task);


  void init( unsigned first_proc, unsigned last_proc, char* sem_name, Task& task);

  //TODO - necessary once we eventually rewrite execute and team creation code
  //void initialSync();

  //Making this static is really ugly, but I don't have time to rewrite all of
  //parallel.c and team.c right now. Currently calls straight into the Task
  //class to figure out what work to do.
  static void execute(void* in_arg);
private:

  //std::vector< std::vector< Runnable > > schedule; //each piece of work to do
                                                   

};


#endif //ifndef _RTOMP_RUNTIME_H_
