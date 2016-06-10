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


#include <iostream>
#include <time.h>
#include "Constants.h"

void get_time( timespec& ts ){
	clock_gettime( CLOCK_MONOTONIC, &ts);
}

bool operator>(const timespec& lhs, const timespec& rhs){
  if(lhs.tv_sec > rhs.tv_sec){
    return true;
  }

  if(rhs.tv_sec > lhs.tv_sec){
    return false;
  }

  if(lhs.tv_nsec > rhs.tv_nsec){
    return true;
  }
  return false;
}

bool operator<(const timespec& lhs, const timespec& rhs){
	if(lhs.tv_sec < rhs.tv_sec){
    return true;
  }

  if(rhs.tv_sec < lhs.tv_sec){
    return false;
  }

  if(lhs.tv_nsec < rhs.tv_nsec){
    return true;
  }
  return false;
}

bool operator==(const timespec& lhs, const timespec& rhs){
  if ((lhs.tv_nsec == rhs.tv_nsec) && (lhs.tv_sec == rhs.tv_sec)){
    return true;
  }
  return false;
}

bool operator<=(const timespec& lhs, const timespec& rhs){
  if((lhs < rhs) || (lhs == rhs)){
    return true;
  }
  return false;
}

void ts_diff ( const timespec& ts1, const timespec& ts2, timespec& result){
	if( ts1 > ts2 ){
		result.tv_nsec = ts1.tv_nsec - ts2.tv_nsec;
		result.tv_sec = ts1.tv_sec - ts2.tv_sec;
	}	else {
		result.tv_nsec = ts2.tv_nsec - ts1.tv_nsec;
		result.tv_sec = ts2.tv_sec - ts1.tv_sec;
	}

	if( result.tv_nsec < 0 ){
		result.tv_nsec += nanosec_in_sec;
		result.tv_sec -= 1;
	}
}

void ts_add ( const timespec &ts1, const timespec &ts2, timespec& result){
	result.tv_nsec = ts1.tv_nsec + ts2.tv_nsec;
	result.tv_sec = ts1.tv_sec + ts2.tv_sec;

	if( result.tv_nsec >= nanosec_in_sec ){
		result.tv_nsec -= nanosec_in_sec;
		result.tv_sec += 1;
	}
}

void ts_int_div (timespec& ts, unsigned divisor){
	if (divisor < 2)
		return;

	long int nsec_div = ts.tv_nsec / divisor;
	float nsec_rem = (ts.tv_nsec % divisor) / divisor;
	if( nsec_rem >= 0.5 )
		nsec_div++;

	unsigned sec_div = ts.tv_sec / divisor;
	unsigned sec_rem = ts.tv_sec % divisor;
	unsigned nsec_from_sec_rem = (nanosec_in_sec / divisor) * sec_rem;

	ts.tv_sec = sec_div;
	ts.tv_nsec = nsec_div + nsec_from_sec_rem;
	while( ts.tv_nsec >= nanosec_in_sec ){
		ts.tv_sec += 1;
		ts.tv_nsec -= nanosec_in_sec;
	}
}

void sleep_until_future_ts (const timespec& end_time){
	timespec curr_time;
	get_time(curr_time);

	//If we have already passed end_time, then return immediately
	if( curr_time > end_time )
		return;

	//Otherwise, nanosleep
	timespec wait;
	ts_diff(curr_time, end_time, wait);

	//The function nanosleep can return before it's supposed to, so we verify it
	while( nanosleep(&wait, &wait) != 0 ){
		//On at least one occasion the function did not return when it should have
		//so we double check while in this loop. 
		if ((wait.tv_sec == 0) && (wait.tv_nsec < 0))
			break;
		if ((wait.tv_sec == 0) && (wait.tv_nsec == 0))
			break;
	}

	return;
}

