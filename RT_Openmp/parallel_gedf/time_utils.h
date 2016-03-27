//David Ferry
//Nov 11, 2011

//Simple utilities for deadling with timespec structs

#ifndef RT_GOMP_TIME_UTILS_H
#define RT_GOMP_TIME_UTILS_H

#include <time.h>

inline void get_time( timespec* ts ){
	clock_gettime( CLOCK_MONOTONIC, ts);
}

//Converts a seconds/nanoseconds timespec into milliseconds
//Passing an unsigned by reference is weird, but keeps the notation consistent
//with the function ms_to_timespec
inline void timespec_to_ms(const timespec& ts, unsigned& ms){
	ms = ts.tv_sec * 1000;
	ms += ts.tv_nsec / 1000000; 
	ms += (ts.tv_nsec % 1000000) > 500000 ? 1 : 0;
}

inline void get_curr_time_ms(unsigned& ms){
	timespec ts;
	get_time(&ts);
	timespec_to_ms(ts,ms);
}

inline unsigned timespec_to_ms(const timespec& ts){
	unsigned ms;
	ms = ts.tv_sec * 1000;
	ms += ts.tv_nsec / 1000000;
	ms += (ts.tv_nsec % 1000000) > 500000 ? 1 : 0;
	return ms;
}

inline unsigned long timespec_to_ns(const timespec& ts){
	unsigned long ns;
	ns = ts.tv_sec * 1000000000;
	ns += ts.tv_nsec;
	return ns;
}

//Converts an unsigned milliseconds value into a timespec
inline void ms_to_timespec( const unsigned ms, timespec& ts){
	ts.tv_sec = ms/1000;
	ts.tv_nsec = (ms % 1000) * 1000000;
}

void print_timespec(const timespec& ts);

bool operator>(const timespec& lhs, const timespec& rhs);

bool operator<(const timespec& lhs, const timespec& rhs);

bool operator==(const timespec& lhs, const timespec& rhs);

bool operator<=(const timespec& lhs, const timespec& rhs);


#endif //End RT_GOMP_TIME_UTILS_H
