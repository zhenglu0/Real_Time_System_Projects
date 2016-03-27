#ifndef RT_GOMP_SINGLE_USE_BARRIER_H
#define RT_GOMP_SINGLE_USE_BARRIER_H

enum rt_gomp_single_use_barrier_error_codes
{
	RT_GOMP_SINGLE_USE_BARRIER_SUCCESS,
	RT_GOMP_SINGLE_USE_BARRIER_INVALID_VALUE_ERROR,
	RT_GOMP_SINGLE_USE_BARRIER_SHM_OPEN_FAILED_ERROR,
	RT_GOMP_SINGLE_USE_BARRIER_FTRUNCATE_FAILED_ERROR,
	RT_GOMP_SINGLE_USE_BARRIER_MMAP_FAILED_ERROR
};

int init_single_use_barrier(const char *name, unsigned value);
int await_single_use_barrier(const char *name);

#endif /* RT_GOMP_SINGLE_USE_BARRIER_H */
