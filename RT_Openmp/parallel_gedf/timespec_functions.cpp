#include "timespec_functions.h"
#include <math.h>
//#include <stdio.h>

// Prints out the timespec as a single decimal number in seconds.
std::ostream& operator<<(std::ostream & stream, const timespec & ts){
	stream << ts.tv_sec << ".";
	long nsec = ts.tv_nsec;
	for (unsigned i = 0; i < 9; ++i, nsec /= 10)
	{
		if (nsec == 0)
		{
			stream << "0";
		}
	}
	
	stream << ts.tv_nsec;
	return stream;
}

//Returns the sum of two timespec structs. Does no bounds checking.
timespec operator+(const timespec & ts1, const timespec & ts2){
  timespec result;
  result.tv_nsec = ts1.tv_nsec + ts2.tv_nsec;
  result.tv_sec = ts1.tv_sec + ts2.tv_sec;
  if( result.tv_nsec >= nanosec_in_sec ){
    result.tv_nsec -= nanosec_in_sec;
    result.tv_sec += 1;
  }
  return result;
}

timespec operator*(const timespec & ts, double scalar)
{
	long nsecs = ts.tv_nsec + nanosec_in_sec * ts.tv_sec;
	nsecs = static_cast<long>(nsecs * scalar);
	timespec result = { nsecs / nanosec_in_sec, nsecs % nanosec_in_sec };
	return result;
}

timespec operator*(double scalar, const timespec & ts)
{
	return ts * scalar;
}

timespec operator/(const timespec & ts, double scalar)
{
	long nsecs = ts.tv_nsec + nanosec_in_sec * ts.tv_sec;
	nsecs = static_cast<long>(nsecs / scalar);
	timespec result = { nsecs / nanosec_in_sec, nsecs % nanosec_in_sec };
	return result;
}

double operator/(const timespec & ts1, const timespec & ts2)
{
	long ts1_nsecs = ts1.tv_nsec + nanosec_in_sec * ts1.tv_sec;
	long ts2_nsecs = ts2.tv_nsec + nanosec_in_sec * ts2.tv_sec;
	return static_cast<double>(ts1_nsecs) / ts2_nsecs;
}

//Takes the difference between two timespec structs and stores the result in
//the specified timespec. Does no bounds checking. The result is always
//a positive time value. 
void ts_diff (timespec& ts1, timespec& ts2, timespec& result){
  if( ts1 > ts2 ){
    result.tv_nsec = ts1.tv_nsec - ts2.tv_nsec;
    result.tv_sec = ts1.tv_sec - ts2.tv_sec;
  } else {
    result.tv_nsec = ts2.tv_nsec - ts1.tv_nsec;
    result.tv_sec = ts2.tv_sec - ts1.tv_sec;
  }

  if( result.tv_nsec < 0 ){            //If we have a negative nanoseconds
    result.tv_nsec += nanosec_in_sec; //value then we carry over from the
    result.tv_sec -= 1;               //seconds part of the timespec
  }
}

//This function will take a timespec value and nanosleep until then
void sleep_until_ts (timespec& end_time){
  //Get current time
  timespec curr_time;
  get_time(&curr_time);

	//If we have already passed end_time, then return immediately
	if( curr_time > end_time )
		return;

  //Otherwise, nanosleep
	timespec wait;
	ts_diff(curr_time, end_time, wait);

	while( nanosleep(&wait,&wait) != 0 ){
		if ((wait.tv_sec == 0) && (wait.tv_nsec < 0))
			break;
		if ((wait.tv_sec == 0) && (wait.tv_nsec == 0))
			break;
		}

	return;
}

void sleep_for_ts (timespec& sleep_time){
  //Otherwise, nanosleep
    timespec zero = { 0, 0 };
	while( nanosleep(&sleep_time,&sleep_time) != 0 )
	{
		if (sleep_time <= zero) break;
	}

	return;
}

/*
 * 1ms workload on a specific machine
 * You need to tune this value or change workload
 */
static inline void
workload_for_1micros(int length) {
    double temp = 0;
    long long i;
        for ( i = 0; i < 52*length; ++i )
        temp = sqrt((double)i*i);
}

void busy_work(timespec length)
{
	timespec curr_time;
	timespec twoms = { 0, 2000000 };
	timespec twomicros = { 0, 1000 };
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &curr_time);
	timespec target_time = curr_time + length;
	while(curr_time+twoms < target_time)
	{
		workload_for_1micros(1000);
		clock_gettime(CLOCK_THREAD_CPUTIME_ID, &curr_time);
	}
	while(curr_time+twomicros < target_time)
	{
		workload_for_1micros(1);
		clock_gettime(CLOCK_THREAD_CPUTIME_ID, &curr_time);
	}
}

void busy_work2(timespec length)
{
        //timespec curr_time, curr_time2, time3;
        //clock_gettime(CLOCK_THREAD_CPUTIME_ID, &curr_time);
	//time3 = curr_time + length;
	long long iterations = length.tv_sec*1000000+length.tv_nsec/((long) 1000);
	long long i;
	for ( i = 0; i < iterations; ++i )
		workload_for_1micros(1);
        //clock_gettime(CLOCK_THREAD_CPUTIME_ID, &curr_time2);
	//if (curr_time2 > time3){
	//	printf("%lld : %lld     %lld : %lld \n",curr_time2.tv_sec,curr_time2.tv_nsec, time3.tv_sec,time3.tv_nsec);
	//	exit(1);
	//}
}

void busy_work3(timespec length)
{
        timespec curr_time;
        timespec twoms = { 0, 1000000 };
        timespec twomicros = { 0, 1000 };
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &curr_time);
        timespec target_time = curr_time + length;
        while(curr_time+twoms < target_time)
        {
                workload_for_1micros(1000);
                clock_gettime(CLOCK_THREAD_CPUTIME_ID, &curr_time);
        }
	timespec timelength;
	ts_diff(curr_time, target_time, timelength);
        long long iterations = timelength.tv_sec*1000000+timelength.tv_nsec/((long) 1000);
        long long i;
        for ( i = 0; i < iterations; ++i )
                workload_for_1micros(1);
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

bool operator>=(const timespec& lhs, const timespec& rhs){
  if((lhs > rhs) || (lhs == rhs)){
    return true;
  }
  return false;
}

bool operator!=(const timespec& lhs, const timespec& rhs){
  if(!(lhs == rhs)){
    return true;
  }
  return false;
}

