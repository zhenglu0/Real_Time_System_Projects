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


#include "Task.h"

Task::Task( const unsigned task_id, const char* name, const timespec& period,
            const unsigned iterations){
  this->task_id = task_id;
	if(name != NULL)
  	this->name = std::string( name );
	else
		this->name = std::string("");
  this->period = period;
  this->iterations = iterations;

  period_misses = std::vector< unsigned > ( iterations, 0 );
}

Task::~Task(){}//No dynamic allocation

void Task::addSegment( const timespec& exec_length, const timespec& release,
                       const unsigned priority){
  segments.push_back( Segment( exec_length, release, priority ) );
}

void Task::addStrandToLastSeg( void (*func) (void*), const void* data,
                               const unsigned CPU){
	segments.back().addStrand( func, data, CPU );
}

void Task::printMisses(){
  for(unsigned i = 0; i < iterations; i++){
    printf("Task %d (%s): period %d missed %d\n",task_id,name.c_str(),i,period_misses[i]);
	}
}

unsigned Task::getMisses(){  
  unsigned ret = 0;
  for(unsigned i = 0; i < iterations; i++)
		if(period_misses[i] > 0)
      ret++;

  return ret;
}
