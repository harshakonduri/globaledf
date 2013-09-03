/**
 * @file
 *
 * @brief Global EDF Scheduler API
 *
 * @ingroup ScoreSchedulerGlobalEDF
 */

/*
 *  Copyright (C) 2011 On-Line Applications Research Corporation (OAR).
 *
 *  Copyright (c) 2013 embedded brains GmbH.
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 */

#ifndef _RTEMS_SCORE_SCHEDULERSIMPLE_SMP_H
#define _RTEMS_SCORE_SCHEDULERGLOBALEDF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <rtems/score/scheduler.h>
#include <rtems/score/schedulersimple.h>
#include <rtems/score/schedulerpriority.h>
#include <rtems/score/scheduleredf.h>

/**
 * @defgroup ScoreSchedulerGlobalEDF Global EDF Scheduler
 *
 * @ingroup ScoreScheduler
 *
 * The Simple SMP Scheduler allocates a processor for the processor count
 * highest priority ready threads.  The thread priority and position in the
 * ready chain are the only information to determine the scheduling decision.
 * Threads with an allocated processor are in the scheduled chain.  After
 * initialization the scheduled chain has exactly processor count nodes.  Each
 * processor has exactly one allocated thread after initialization.  All
 * enqueue and extract operations may exchange threads with the scheduled
 * chain.  One thread will be added and another will be removed.  The scheduled
 * and ready chain is ordered according to the thread priority order.  The
 * chain insert operations are O(count of ready threads), thus this scheduler
 * is unsuitable for most real-time applications.
 *
 * The thread preempt mode will be ignored.
 *
 * @{
 */

typedef enum {
    THREAD_IN_READY_QUEUE, 
    THREAD_IN_SCHEDULED_QUEUE,
  } THREAD_PLACED_IN;

typedef struct {
  /* Ready Queue using the RedBlack Tree Structure*/
  RBTree_Control ready;
  /* Chain Control that uses the scheduled tasks */
  Chain_Control scheduled;
  /* Use an integer to identify whether a task is on ready or scheduled queues */
  /* THREAD LOCATION IDENTIFICATION*/
  THREAD_PLACED_IN thread_location;

  /* The state of tasks in the queue */
  Scheduler_EDF_Queue_state queue_state;

  /* RBTree Node information*/
  RBTree_Node node;

} Scheduler_globaledf_Control;

  typedef struct {
  /* Use an integer to identify whether a task is on ready or scheduled queues */
  /* THREAD LOCATION IDENTIFICATION*/
  THREAD_PLACED_IN thread_location;

  /* The state of tasks in the queue */
  Scheduler_EDF_Queue_state queue_state;

  /* RBTree Node information*/
  RBTree_Node Node;  /* Use an integer to identify whether a task is on ready or scheduled queues */

    /* Thread information of every thread*/
  Thread_Control *thread;

  } Scheduler_globaledf_perthread;

/**
 * @brief Entry points for the Global EDF Scheduler.
 */
#define SCHEDULER_GLOBALEDF_ENTRY_POINTS \
  { \
    _Scheduler_globaledf_Initialize, \
    _Scheduler_globaledf_Schedule, \
    _Scheduler_globaledf_Yield, \
    _Scheduler_globaledf_Extract, \
    _Scheduler_globaledf_Enqueue_priority_fifo, \
    _Scheduler_globaledf_Allocate, \
    _Scheduler_EDF_Free, \
    _Scheduler_EDF_Update, \
    _Scheduler_globaledf_Enqueue_priority_fifo, \
    _Scheduler_globaledf_Enqueue_priority_lifo, \
    _Scheduler_globaledf_Extract, \
    _Scheduler_EDF_Priority_compare, \
    _Scheduler_EDF_Release_job, \
    _Scheduler_default_Tick, \
    _Scheduler_default_Start_idle \
  }

void _Scheduler_globaledf_Initialize( void );

void _Scheduler_globaledf_Enqueue_priority_fifo( Thread_Control *thread );

void _Scheduler_globaledf_Enqueue_priority_lifo( Thread_Control *thread );

void _Scheduler_globaledf_Extract( Thread_Control *thread );

void *_Scheduler_globaledf_Allocate( Thread_Control *thread);// added Allocate to smp scheduler.

void _Scheduler_globaledf_Yield( Thread_Control *thread );

void _Scheduler_globaledf_Schedule( Thread_Control *thread );

void _Scheduler_globaledf_Start_idle(
  Thread_Control *thread,
  Per_CPU_Control *cpu
);

/** @} */

#ifdef __cplusplus
}
#endif

#endif
/* end of include file */
