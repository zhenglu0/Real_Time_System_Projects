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


#ifndef _RTOMP_SEGMENT_H_
#define _RTOMP_SEGMENT_H_

#include <time.h>
#include <vector>

#include "Strand.h"
#include "Constants.h"

class Segment {
friend class Runtime;

public:

	//Constructors and destructors
  Segment( const timespec& exec_length, const timespec& release, 
           const unsigned priority);
  ~Segment();

 /* Adds a strand to the segment, must know what function to exec, the data to
 *  use, and the CPU to execute on.
 */
	void addStrand( void (*func) (void*), const void* data, const unsigned CPU);

private:

  timespec exec_length;   // Execution length of segment
  timespec release;       // Relative release time from start of period

  unsigned num_subtasks;  // Number of subtasks in the segment
  std::vector< std::vector< Strand > > strands; 
                          // A set of vectors to strands. Each vector of strands
                          // corresonds to one processor, so each CPU can access
                          // its work quickly. For example, strands[0] would be
                          // a vector of all strands that cpu 0 has to compute
                          // in this segment. 

  unsigned priority;      // Priority of the segment- note that this value is
                          // passed to the OS scheduler without modification
};

#endif //ifndef _RTOMP_SEGMENT_H_
