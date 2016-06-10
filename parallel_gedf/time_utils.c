//David Ferry
//Apr 1, 2012

#include <iostream>
#include <time.h>

void print_timespec( const timespec& ts){
	std::cout << "Seconds:  " << ts.tv_sec << std::endl;
	std::cout << "NSeconds: " << ts.tv_nsec << std::endl;
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



