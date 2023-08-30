/*
 * stack.c
 *
 *  Created on: 18 Oct 2011
 *  Copyright 2011 Nicolas Melot
 *
 * This file is part of TDDD56.
 * 
 *     TDDD56 is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     TDDD56 is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 * 
 *     You should have received a copy of the GNU General Public License
 *     along with TDDD56. If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#ifndef DEBUG
#define NDEBUG
#endif

#include <assert.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "stack.h"
#include "non_blocking.h"

#if NON_BLOCKING == 0
#warning Stacks are synchronized through locks
#else
#if NON_BLOCKING == 1
#warning Stacks are synchronized through hardware CAS
#else
#warning Stacks are synchronized through lock-based CAS
#endif
#endif

stack_t global_array[5000]; // MAX_PUSH_POP is 5000
pthread_mutex_t our_lock;


void init_lock()
{
    if (pthread_mutex_init(&our_lock, NULL) != 0)
		{
			printf("\n mutex init failed\n");
			return 1;
		}
}

void lock()
{
  pthread_mutex_lock(&our_lock);
}

void unlock()
{
  pthread_mutex_unlock(&our_lock);
}

int
stack_check(stack_t **head, int push_counter)
{
// Do not perform any sanity check if performance is bein measured
#if MEASURE == 0
	// Use assert() to check if your stack is in a state that makes sens
	// This test should always pass 
  int counter=0;
  while((*head)->next!=NULL)
  {
    counter++;
    *head = (*head)->next;
  }
  //printf("counter -%d\n", counter);
  //printf("push_counter -%d\n", push_counter);
	assert(counter == push_counter);

	// This test fails if the task is not allocated or if the allocation failed
	assert((*head) != NULL);
#endif
	// The stack is always fine
	return 1;
}

void /* Return the type you prefer */
stack_push(int value, stack_t **this)
{
  //printf("stack push is called!!\n");
#if NON_BLOCKING == 0
  // Implement a lock_based stack
    lock();
    stack_t* new_top;
    for(int i = 0; i<5000;i++)
    {
      if(global_array[i].occupied==0)
      {
        new_top = &global_array[i];
        break;
      }
    }
    new_top->next=*this;
    new_top->value = value;
    new_top->occupied = 1;
    *this = new_top;
    //printf("pushing the value %d into the stack\n",value);
    unlock();
  
#elif NON_BLOCKING == 1
  // Implement a harware CAS-based stack
  //we do 2 compare and swap
  stack_t* new_top=NULL;
    int found = 0;
    int def = 0;
    while(!found)
    {
      for(int i = 0; i<5000;i++)
      {
        new_top = &global_array[i];
        int result = cas(&(new_top->occupied), 0, 1); //occupied = 0, if old 
        if(result == 0){         
          found = 1;
          break;
        }
      }
      stack_t *old;
      new_top->value = value;
      do{
        old = *this;
        new_top->next = old;
      }while(cas(this, old, new_top) != old);
    }

#else
  /*** Optional ***/
  // Implement a software CAS-based stack
  stack_t* new_top=NULL;
    int found = 0;
    int def = 0;
    while(!found)
    {
      for(int i = 0; i<5000;i++)
      {
        new_top = &global_array[i];
        int result = software_cas(&(new_top->occupied), 0, 1, &our_lock); //occupied = 0, if old 
        if(result == 0){         
          found = 1;
          break;
        }
      }
      stack_t *old;
      new_top->value = value;
      do{
        old = *this;
        new_top->next = old;
      }while(software_cas(this, old, new_top, &our_lock) != old);
    }
  

#endif

  // Debug practice: you can check if this operation results in a stack in a consistent check
  // It doesn't harm performance as sanity check are disabled at measurement time
  // This is to be updated as your implementation progresses

  //mutex lock is needed

  //stack_check((stack_t*)1);   

}

stack_t* /* Return the type you prefer */
stack_pop(stack_t** this)
{
#if NON_BLOCKING == 0
  lock();
  
  stack_t *old = *this;
  if(old==NULL)
  {
    unlock();
    return NULL;
  }
  old->occupied=0;
  *this = (*this)->next;
  //printf("popping the value %d from the stack\n",old->value);
  unlock();
  return old;
  
#elif NON_BLOCKING == 1
  // Implement a harware CAS-based stack
  stack_t *old,*next;
  do{
    old = *this;
    if(old == NULL)
    { 
      return NULL;
    }
    next = (*this)->next;
  }while(cas(this, old, next)!=old);
  old->occupied = 0;
  //printf("popping the value %d from the stack\n",old->value);
  return old;
#else
  /*** Optional ***/
  stack_t *old,*next;
  do{
    old = *this;
    if(old == NULL)
    { 
      return NULL;
    }
    next = (*this)->next;
  }while(software_cas(this, old, next, &our_lock)!=old);
  //printf("popping the value %d from the stack\n",old->value);
  old->occupied = 0;
  return 1;
#endif

  return 0;
}

