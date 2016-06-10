// A simple matrix-vector multiplication task that uses OpenMP
// Call the functions with the following arguments in argv: executable_name num_rows num_cols

#include <omp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <iostream>
#include "task.h"

size_t M, N;
double *matrix_1D, *vector, *result;

enum rt_gomp_simple_task_error_codes
{
	RT_GOMP_SIMPLE_TASK_SUCCESS,
	RT_GOMP_SIMPLE_TASK_INVALID_ARGUMENTS,
	RT_GOMP_SIMPLE_TASK_MEM_ALLOC_ERROR
};

int init(int argc, char *argv[])
{
	// Read the matrix and vector sizes
	if (!( 
		argc == 3 &&
		std::istringstream(argv[1]) >> M &&
		std::istringstream(argv[2]) >> N
	))
	{
		fprintf(stderr, "ERROR: Invalid initialization arguments");
		return RT_GOMP_SIMPLE_TASK_INVALID_ARGUMENTS;
	}
	
	// Allocate memory for the matrix and vectors
	matrix_1D = new (std::nothrow) double[M*N];
	if (!matrix_1D)
	{
		fprintf(stderr, "ERROR: Memory allocation failed");
		return RT_GOMP_SIMPLE_TASK_MEM_ALLOC_ERROR;
	}
	// Set all the value of matrix_1D to 1
	for (size_t i = 0; i < M*N; i++)
	{
		matrix_1D[i] = 1;	
		//std::cout << matrix_1D[i] << " ";
	}
	//std::cout << std::endl;

	vector = new (std::nothrow) double[N];
	if (!vector)
	{
		fprintf(stderr, "ERROR: Memory allocation failed");
		return RT_GOMP_SIMPLE_TASK_MEM_ALLOC_ERROR;
	}
	// Set all the value of matrix_1D to 1
	for (size_t i = 0; i < N; i++)
	{
		vector[i] = 2;
		//std::cout << vector[i] << " ";
	}
	//std::cout << std::endl;

	result = new (std::nothrow) double[M];
	if (!result)
	{
		fprintf(stderr, "ERROR: Memory allocation failed");
		return RT_GOMP_SIMPLE_TASK_MEM_ALLOC_ERROR;
	}	
	
	return 0;
}

int run(int argc, char *argv[])
{
	//double (*matrix_2D)[N] = reinterpret_cast<double (*)[N]>(matrix_1D);
	// Perform matrix-vector multiplication
	#pragma omp parallel for
	for (size_t r = 0; r < M; ++r)
	{
		result[r] = 0;
		for (size_t c = 0; c < N; ++c)
		{
			result[r] += matrix_1D[r*M+c] * vector[c];
		}
	}
	
	return 0;
}

int finalize(int argc, char *argv[])
{
	//for (size_t i = 0; i < M; i++)
		//std::cout << result[i] << " ";
	//std::cout << std::endl;
	delete[] matrix_1D;
	delete[] vector;
	delete[] result;
	return 0;
}

task_t task = { init, run, finalize };

