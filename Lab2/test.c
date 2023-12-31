/*
 * test.c
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
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 * 
 *     You should have received a copy of the GNU General Public License
 *     along with TDDD56. If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <stddef.h>

#include "test.h"
#include "stack.h"
#include "non_blocking.h"
#include <stdlib.h>

#define test_run(test)\
  printf("[%s:%s:%i] Running test '%s'... ", __FILE__, __FUNCTION__, __LINE__, #test);\
  test_setup();\
  if(test())\
  {\
    printf("passed\n");\
  }\
  else\
  {\
    printf("failed\n");\
  }\
  test_teardown();

/* Helper function for measurement */
double timediff(struct timespec *begin, struct timespec *end)
{
	double sec = 0.0, nsec = 0.0;
   if ((end->tv_nsec - begin->tv_nsec) < 0)
   {
      sec  = (double)(end->tv_sec  - begin->tv_sec  - 1);
      nsec = (double)(end->tv_nsec - begin->tv_nsec + 1000000000);
   } else
   {
      sec  = (double)(end->tv_sec  - begin->tv_sec );
      nsec = (double)(end->tv_nsec - begin->tv_nsec);
   }
   return sec + nsec / 1E9;
}

typedef int data_t;
#define DATA_SIZE sizeof(data_t)
#define DATA_VALUE 5

#ifndef NDEBUG
int
assert_fun(int expr, const char *str, const char *file, const char* function, size_t line)
{
	if(!(expr))
	{
		fprintf(stderr, "[%s:%s:%zu][ERROR] Assertion failure: %s\n", file, function, line, str);
		abort();
		// If some hack disables abort above
		return 0;
	}
	else
		return 1;
}
#endif

stack_t *stack;
stack_t **head;
data_t data;
int push_counter = 0;
int pop_counter = 0;

#if MEASURE != 0
struct stack_measure_arg
{
  int id;
};
typedef struct stack_measure_arg stack_measure_arg_t;

struct timespec t_start[NB_THREADS], t_stop[NB_THREADS], start, stop;

#if MEASURE == 1
void* stack_measure_pop(void* arg)
  {
    stack_measure_arg_t *args = (stack_measure_arg_t*) arg;
    int i;
    printf("stack measure pop is called! this is for debug \n");

    clock_gettime(CLOCK_MONOTONIC, &t_start[args->id]);
    for (i = 0; i < MAX_PUSH_POP / NB_THREADS; i++)
    //for (i = 0; i < 4; i++)
      {
        // See how fast your implementation can pop MAX_PUSH_POP elements in parallel
#if NON_BLOCKING == 0   
        //int* val_pointer=NULL;
        stack_t *old = stack_pop(head);
        if(old ==NULL)
          break;      //just quality of life
#elif NON_BLOCKING == 1
        stack_t *old = stack_pop(head);
        if(old == NULL)
          break;
#else
        stack_t *old = stack_pop(head);
        if(old == NULL)
          break;
#endif
      }
    //printf("ENDDDD DDD\n");
    clock_gettime(CLOCK_MONOTONIC, &t_stop[args->id]);

    return NULL;
  }
#elif MEASURE == 2
void*
stack_measure_push(void* arg)
{
  stack_measure_arg_t *args = (stack_measure_arg_t*) arg;
  int i;
  //TODO
  //printf("stack measure push is called! this is for debug \n");
  //printf("stack head before push - %p\n", head);
  //printf("stack *head before push - %p\n", *head);
  clock_gettime(CLOCK_MONOTONIC, &t_start[args->id]);
  for (i = 0; i < MAX_PUSH_POP / NB_THREADS; i++)
  //for (i = 0; i < 2; i++)
    {

#if NON_BLOCKING == 0   
        // See how fast your implementation can push MAX_PUSH_POP elements in parallel
        //lock
        int value = rand();
        stack_push(value, head);
#elif NON_BLOCKING == 1
        // TODO check 
        int value = rand();
        //printf("pushed values %d\n",value);
        stack_push(value, head);
#else
        int value = rand();
        //printf("pushed values %d\n",value);
        stack_push(value, head);
#endif
    }
    //printf("For push has ended\n");
  clock_gettime(CLOCK_MONOTONIC, &t_stop[args->id]);
  return NULL;
}
#endif
#endif

/* A bunch of optional (but useful if implemented) unit tests for your stack */
void
test_init()
{
  // Initialize your test batch
}

void
test_setup()
{
  // Allocate and initialize your test stack before each test
  //data = DATA_VALUE;

  // Allocate a new stack and reset its values
  stack = malloc(sizeof(stack_t));
  head = &stack;
  // Reset explicitely all members to a well-known initial value
  // For instance (to be deleted as your stack design progresses):
  stack->value = 1;
}

void
test_teardown()
{
  // Do not forget to free your stacks after each test
  // to avoid memory leaks
  free(stack);
}

void
test_finalize()
{
  // Destroy properly your test batch
}

void* test_stackPush()
{
  for (int i = 0; i < MAX_PUSH_POP / NB_THREADS; i++)
    {
        int value = rand();
        stack_push(value, head);
        lock();
        push_counter++;
        unlock();
    }
}

void* test_stackPop()
{
  for (int i = 0; i < MAX_PUSH_POP / NB_THREADS; i++)
    {
        int value = rand();
        stack_t *old = stack_pop(head);
        if(old == NULL)
          break;
        lock();
        pop_counter++;
        unlock();
    }
}

int
test_push_safe()
{
  pthread_t thread[NB_THREADS];
  pthread_attr_t attr;
  pthread_attr_init(&attr);

  

  


  // Make sure your stack remains in a good state with expected content when
  // several threads push concurrently to it

  // Do some work // TODO test case if you want to
  for (int i = 0; i < NB_THREADS; i++)
      pthread_create(&thread[i], &attr, test_stackPush, NULL);

  for (int i = 0; i < NB_THREADS; i++)
      pthread_join(thread[i], NULL);

  stack_t **testHead = &(*head);
  // check if the stack is in a consistent state
  int res = assert(stack_check(testHead, push_counter));

  // check other properties expected after a push operation
  // (this is to be updated as your stack design progresses)
  // Now, the test succeeds
  return res;// && assert(stack->value == 0);
}

int
test_pop_safe()
{
  pthread_t thread[NB_THREADS];
  pthread_attr_t attr;
  pthread_attr_init(&attr);

  
  for (int i = 0; i < NB_THREADS; i++)
      pthread_create(&thread[i], &attr, test_stackPop, NULL);

  for (int i = 0; i < NB_THREADS; i++)
      pthread_join(thread[i], NULL);

  printf("push_counter - %d\n", push_counter);
  printf("pop_counter - %d\n", pop_counter);
  return pop_counter==push_counter;
}

// 3 Threads should be enough to raise and detect the ABA problem
#define ABA_NB_THREADS 3

int
test_aba()
{
#if NON_BLOCKING == 1 || NON_BLOCKING == 2
  int success, aba_detected = 0;
  // Write here a test for the ABA problem
  success = aba_detected;
  return success;
#else
  // No ABA is possible with lock-based synchronization. Let the test succeed only
  return 1;
#endif
}

// We test here the CAS function
struct thread_test_cas_args
{
  int id;
  size_t* counter;
  pthread_mutex_t *lock;
};
typedef struct thread_test_cas_args thread_test_cas_args_t;

void*
thread_test_cas(void* arg)
{
#if NON_BLOCKING != 0
  thread_test_cas_args_t *args = (thread_test_cas_args_t*) arg;
  int i;
  size_t old, local;

  for (i = 0; i < MAX_PUSH_POP; i++)
    {
      do {
        old = *args->counter;
        local = old + 1;
#if NON_BLOCKING == 1
      } while (cas(args->counter, old, local) != old);
#elif NON_BLOCKING == 2
      } while (software_cas(args->counter, old, local, args->lock) != old);
#endif
    }
#endif

  return NULL;
}

// Make sure Compare-and-swap works as expected
int
test_cas()
{
#if NON_BLOCKING == 1 || NON_BLOCKING == 2
  pthread_attr_t attr;
  pthread_t thread[NB_THREADS];
  thread_test_cas_args_t args[NB_THREADS];
  pthread_mutexattr_t mutex_attr;
  pthread_mutex_t lock;

  size_t counter;

  int i, success;

  counter = 0;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE); 
  pthread_mutexattr_init(&mutex_attr);
  pthread_mutex_init(&lock, &mutex_attr);

  for (i = 0; i < NB_THREADS; i++)
    {
      args[i].id = i;
      args[i].counter = &counter;
      args[i].lock = &lock;
      pthread_create(&thread[i], &attr, &thread_test_cas, (void*) &args[i]);
    }

  for (i = 0; i < NB_THREADS; i++)
    {
      pthread_join(thread[i], NULL);
    }

  success = assert(counter == (size_t)(NB_THREADS * MAX_PUSH_POP));

  if (!success)
    {
      printf("Got %ti, expected %i. ", counter, NB_THREADS * MAX_PUSH_POP);
    }

  return success;
#else
  return 1;
#endif
}

int main(int argc, char **argv)
{
  printf("main called \n");
setbuf(stdout, NULL);
// init operations TODO
  srand(time(NULL));
  init_lock();
// MEASURE == 0 -> run unit tests
#if MEASURE == 0
  test_init();

  test_run(test_cas);

  test_run(test_push_safe);
  test_run(test_pop_safe);
  test_run(test_aba);

  test_finalize();
#else
  head = &stack;
  int i;
  //head = &stack;
  pthread_t thread[NB_THREADS];
  pthread_attr_t attr;
  stack_measure_arg_t arg[NB_THREADS];
  pthread_attr_init(&attr);
  clock_gettime(CLOCK_MONOTONIC, &start);
#if MEASURE == 1
      //push simulation MAX_PUSH_POP
      for(int i = 0; i<MAX_PUSH_POP;i++)
      {
        //for cas
        int value = rand();
        stack_push(value, head);
      }
#endif
  for (i = 0; i < NB_THREADS; i++)
    {
      arg[i].id = i;
#if MEASURE == 1
      
      pthread_create(&thread[i], &attr, stack_measure_pop, (void*)&arg[i]);
#else
      printf("calling the push function\n");
      pthread_create(&thread[i], &attr, stack_measure_push, (void*)&arg[i]);
#endif
    }

  for (i = 0; i < NB_THREADS; i++)
    {
      pthread_join(thread[i], NULL);
    }
  clock_gettime(CLOCK_MONOTONIC, &stop);

  // Print out results
  for (i = 0; i < NB_THREADS; i++)
    {
        printf("Thread %d time: %f\n", i, timediff(&t_start[i], &t_stop[i]));
    }
#endif

  return 0;
}
