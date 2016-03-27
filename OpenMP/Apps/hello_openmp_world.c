#include <stdio.h>
#ifdef _OPENMP
#include <omp.h>
#endif

int main()
{
    #ifdef _OPENMP
        printf("Number of processors: %d\n", omp_get_num_procs());
        //omp_set_num_threads(8);
    #pragma omp parallel
    {
        printf("This is thread: %3d out of %3d\n", 
	       omp_get_thread_num(), omp_get_num_threads());
    }
    printf("Back to serial. Done.");
    #else
        printf("OpenMP support is not available.");
    #endif
    return 0;
}
