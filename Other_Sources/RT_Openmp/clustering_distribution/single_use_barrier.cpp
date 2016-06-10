#include "single_use_barrier.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <cstdlib>
#include <errno.h>
#include <time.h>

typedef struct
{
	unsigned value;
}
barrier_t;

static volatile barrier_t *get_barrier(const char *name, int *error_flag)
{
	int fd = shm_open(name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	if( fd == -1 )
	{
		perror("ERROR: single_use_barrier call to shm_open failed");
		*error_flag = RT_GOMP_SINGLE_USE_BARRIER_SHM_OPEN_FAILED_ERROR;
		return NULL;
	}

	int ret_val = ftruncate(fd, sizeof(barrier_t));
	if( ret_val == -1 )
	{
		perror("ERROR: single_use_barrier call to ftruncate failed");
		*error_flag = RT_GOMP_SINGLE_USE_BARRIER_FTRUNCATE_FAILED_ERROR;
		return NULL;
	}

	volatile barrier_t *barrier = (barrier_t *) mmap(NULL, sizeof(barrier_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (barrier == MAP_FAILED)
	{
		perror("ERROR: single_use_barrier call to mmap failed");
		*error_flag = RT_GOMP_SINGLE_USE_BARRIER_MMAP_FAILED_ERROR;
		return NULL;
	}
	
	ret_val = close(fd);
	if( ret_val == -1 )
	{
		perror("WARNING: single_use_barrier call to close file descriptor failed\n");
	}
	
	return barrier;
}

static void unmap_barrier(volatile barrier_t *barrier)
{
	int ret_val = munmap((void *) barrier, sizeof(barrier_t));
	if (ret_val == -1)
	{
		perror("WARNING: single_use_barrier call to munmap failed\n");
	}
}

static void destroy_barrier(const char *name)
{
	int ret_val = shm_unlink(name);
	// If the name cannot be found, then the calling process lost the race 
	// to destroy the barrier which is not a problem. Report any other errors.
	if (ret_val == -1 && errno != ENOENT)
	{
		perror("WARNING: single_use_barrier call to shm_unlink failed\n");
	}
}

int init_single_use_barrier(const char *name, unsigned value)
{
	if (value == 0)
	{
		fprintf(stderr, "ERROR: A barrier cannot be created for zero tasks");
		return RT_GOMP_SINGLE_USE_BARRIER_INVALID_VALUE_ERROR;
	}
	
	int error_flag = 0;
	volatile barrier_t *barrier = get_barrier(name, &error_flag);
	if (error_flag == 0)
	{
		barrier->value = value;
		unmap_barrier(barrier);
	}
	
	return error_flag;
}

int await_single_use_barrier(const char *name)
{
	int error_flag = 0;
	volatile barrier_t *barrier = get_barrier(name, &error_flag);
	if (error_flag == 0)
	{
		// Decrement the value of the barrier
		__sync_add_and_fetch(&(barrier->value), -1);
		
		// Sleep until the value of the barrier reaches zero
		timespec sleep_time = {0, 500000}; // half millisecond
		while (barrier->value > 0){
			nanosleep(&sleep_time, NULL);	
		}
		
		unmap_barrier(barrier);
		// Processes race to destroy the barrier. The race is semantically harmless.
		destroy_barrier(name);
	}
	
	return error_flag;
}
