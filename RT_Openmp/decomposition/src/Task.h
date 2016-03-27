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


#ifndef _RTOMP_TASK_H_
#define _RTOMP_TASK_H_

#include "Segment.h"
#include "Constants.h"
#include <vector>
#include <string>
#include <cstdio>

class Task {
friend class Runtime;

public:

	Task( const unsigned task_id, const char* name, const timespec& period, 
        const unsigned iterations);
	~Task();

  void addSegment( const timespec& exec_length, const timespec& release,
                   const unsigned priority);

	void addStrandToLastSeg( void (*func) (void*), const void* data, 
                           const unsigned CPU);

  void printMisses();

  unsigned getMisses();

private:
  //Task identifiers
  unsigned task_id; //Numerical ID
  std::string name;      //Textual ID 

  //Variables concerning how long the task runs for
  timespec period;      //How long individual periods are
  unsigned iterations;  //How many iterations the task executes for

  std::vector< Segment > segments; // The segments of this task in order of 
                                   //execution

	//These should be aligned at some point in the future
  //Size is equal to the number of periodic iterations
  std::vector< unsigned > period_misses; //The number of periodic misses we get

};

#endif //ifndef _RTOMP_TASK_H_
