#ifndef RT_GOMP_TIMESPEC_FUNCTIONS_H
#define RT_GOMP_TIMESPEC_FUNCTIONS_H

#include <time.h>
#include <iostream>

const long int nanosec_in_sec = 1000000000;
const long int millisec_in_sec = 1000;
const long int nanosec_in_millisec = 1000000;

inline void get_time( timespec* ts ){
	clock_gettime( CLOCK_MONOTONIC, ts);
}

std::ostream& operator<<(std::ostream & stream, const timespec & ts);
timespec operator+(const timespec & ts1, const timespec & ts2);
timespec operator*(const timespec & ts, double scalar);
timespec operator*(double scalar, const timespec & ts);
timespec operator/(const timespec & ts, double scalar);
double operator/(const timespec & ts1, const timespec & ts2);

void ts_diff (timespec& ts1, timespec& ts2, timespec& result);
void sleep_until_ts (timespec& end_time);
void sleep_for_ts (timespec& sleep_time);
void busy_work(timespec length);

bool operator>(const timespec& lhs, const timespec& rhs);
bool operator<(const timespec& lhs, const timespec& rhs);
bool operator==(const timespec& lhs, const timespec& rhs);
bool operator<=(const timespec& lhs, const timespec& rhs);
bool operator>=(const timespec& lhs, const timespec& rhs);
bool operator!=(const timespec& lhs, const timespec& rhs);

#endif /* RT_GOMP_TIMESPEC_FUNCTIONS_H */
