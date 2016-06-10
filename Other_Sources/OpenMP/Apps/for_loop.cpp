#include <omp.h>
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
#pragma omp parallel for
  for (size_t r = 0; r < 10; ++r)
    cout << "r = " << r << endl;
    
  return 0;
}

