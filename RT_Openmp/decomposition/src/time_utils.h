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


//Simple utilities for deadling with timespec structs

#ifndef RT_OMP_TIME_UTILS_H
#define RT_OMP_TIME_UTILS_H

#include <time.h>

void get_time( timespec& ts );

//Finds the difference between the first two timespecs and stores the result
//in the third timespec. Does no bounds checking.
void ts_diff (const timespec& ts1, const timespec& ts2, timespec& result);

//Produces the sum of the first two timespecs and stores the result in the
//third timespec. Does no bounds checking.
void ts_add (const timespec& ts1, const timespec& ts2, timespec& result);

//Divides a timespec value by an integer. Assumes a positive nanoseconds value.
//Inaccurate to a few nanoseconds depending on the divisor.
void ts_int_div (timespec& ts1, unsigned divisor);

//This function will take a timespec value in the future and sleep until then.
//If the provided time is at the present time or in the past then it will
//return immediately.
void sleep_until_future_ts (const timespec& end_time);

bool operator>(const timespec& lhs, const timespec& rhs);

bool operator<(const timespec& lhs, const timespec& rhs);

bool operator==(const timespec& lhs, const timespec& rhs);

bool operator<=(const timespec& lhs, const timespec& rhs);


#endif //End RT_OMP_TIME_UTILS_H
