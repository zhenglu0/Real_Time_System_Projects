/* Copyright (C) 2005, 2008, 2009 Free Software Foundation, Inc.
   Contributed by Richard Henderson <rth@redhat.com>.

   This file is part of the GNU OpenMP Library (libgomp).

   Libgomp is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   Libgomp is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
   FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
   more details.

   Under Section 7 of GPL version 3, you are granted additional
   permissions described in the GCC Runtime Library Exception, version
   3.1, as published by the Free Software Foundation.

   You should have received a copy of the GNU General Public License and
   a copy of the GCC Runtime Library Exception along with this program;
   see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
   <http://www.gnu.org/licenses/>.  */

/* This is a Linux specific implementation of a barrier synchronization
   mechanism for libgomp.  This type is private to the library.  This 
   implementation uses atomic instructions and the futex syscall.  */

#include <limits.h>
#include "wait.h"
#include "bar.h"

void
gomp_barrier_wait_end (gomp_barrier_t *bar, gomp_barrier_state_t state)
{

  if (__builtin_expect ((state & 1) != 0, 0))
    {
      /* Next time we'll be awaiting TOTAL threads again.  */
      bar->awaited = bar->total;
      __sync_synchronize ();;
      bar->generation += 4;
      futex_wake ((int *) &bar->generation, INT_MAX);
    }
  else
    {
      unsigned int generation = state;

      do
	do_wait ((int *) &bar->generation, generation);
      while (bar->generation == generation);
    }
}

void
gomp_barrier_wait (gomp_barrier_t *bar)
{
  gomp_barrier_wait_end (bar, gomp_barrier_wait_start (bar));
}

/* Like gomp_barrier_wait, except that if the encountering thread
   is not the last one to hit the barrier, it returns immediately.
   The intended usage is that a thread which intends to gomp_barrier_destroy
   this barrier calls gomp_barrier_wait, while all other threads
   call gomp_barrier_wait_last.  When gomp_barrier_wait returns,
   the barrier can be safely destroyed.  */

void
gomp_barrier_wait_last (gomp_barrier_t *bar)
{
  gomp_barrier_state_t state = gomp_barrier_wait_start (bar);
  if (state & 1)
    gomp_barrier_wait_end (bar, state);
}

