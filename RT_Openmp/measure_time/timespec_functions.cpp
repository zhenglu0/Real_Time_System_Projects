#include "timespec_functions.h"

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

void busy_work(timespec length)
{
	timespec curr_time;
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &curr_time);
	timespec target_time = curr_time + length;
	while(curr_time < target_time)
	{
		clock_gettime(CLOCK_THREAD_CPUTIME_ID, &curr_time);
	}
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

