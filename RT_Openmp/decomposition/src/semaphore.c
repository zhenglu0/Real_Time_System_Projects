/* Copyright (C) 2012 Washington University in St. Louis - All Rights Reserved
 *
 * This file is part of the RT_OpenMP (Real-Time OpenMP) system.
 *
 * The OpenMP name is a registered trademark of the OpenMP Architecture Review
 * Board (ARB). The RT_OpenMP project aims to adapt the OpenMP API for use in 
 * real-time systems, but the RT_OpenMP project is not affiliated with or 
 * endorsed by the OpenMP ARB. 
 *
 * THIS SOFTWARE IS FREE OF CHARGE AND IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
 * MERCHANTABILITY OR FITNESS FOR ANY PARTICULAR PURPOSE. THE AUTHORS MAKE NO
 * CLAIM THAT THE SOFTWARE WILL PROVIDE ADEQUATE REAL-TIME OPERATION FOR YOUR
 * PARTICULAR APPLICATION. IN NO EVENT SHALL WASHINGTON UNIVERSITY OR THE 
 * AUTHORS OF THIS SOFTWARE BE LIABLE FOR ANY DAMAGES ARISING FROM THE USE OF 
 * THIS PRODUCT.
 *
 * You have the right to use, modify, and distribute this source code subject
 * to the restrictions in the LICENSE file. Some files included in this 
 * distribution are subject to the GPL v3 license, so programs produced using 
 * RT_OpenMP must be licensed under the GPL v3. See the LICENSE file for more 
 * details.
 */


#include "semaphore.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

volatile struct semaphore* getSemaphore(const char *name){
	int fd = shm_open(name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	if( fd == -1 ){
		fprintf(stderr,"ERROR: Launcher call to shm_open failed! Reason: %s\n",strerror(errno));
		abort();
	}

	ftruncate(fd,sizeof(struct semaphore));

	volatile struct semaphore* ret_val = (struct semaphore*)mmap(NULL,sizeof(struct semaphore),PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (ret_val == MAP_FAILED){
		fprintf(stderr,"ERROR: Launcher call to mmap failed! Reason: %s\n",strerror(errno));
		abort();
	}
	
	return ret_val;
}


int decrement(volatile struct semaphore* s){
	return __sync_add_and_fetch(&(s->value),-1);
}
