/**
 * @file
 *
 * @brief Global EDF Scheduler Implementation
 *
 * @ingroup ScoreSchedulerGlobalEDF
 */

/*
 * Copyright (c) 2013 embedded brains GmbH.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rtems.com/license/LICENSE.
 */

#if HAVE_CONFIG_H
  #include "config.h"
#endif

#include <rtems/score/schedulerglobaledf.h>
#include <rtems/score/schedulersimple.h>
#include <rtems/score/schedulerpriority.h>
#include <rtems/score/scheduleredf.h>
//#include <rtems/score/rbtree.h>
static Scheduler_globaledf_Control *_Scheduler_globaledf_Instance( void )
{
  return _Scheduler.information;
}

static int _Scheduler_EDF_RBTree_compare_function
(
  const RBTree_Node* n1,
  const RBTree_Node* n2
)
{
 printk("/n 0 compare \n ");
  Priority_Control value1 = _RBTree_Container_of
    (n1,Scheduler_globaledf_perthread,Node)->thread->current_priority;
  Priority_Control value2 = _RBTree_Container_of
    (n2,Scheduler_globaledf_perthread,Node)->thread->current_priority;
  /*
   * This function compares only numbers for the red-black tree,
   * but priorities have an opposite sense.
   */
  return (-1)*_Scheduler_Priority_compare(value1, value2);
}


void _Scheduler_globaledf_Initialize( void )
{
 printk("1 init \n ");
 printk("Started Init \n");
  Scheduler_globaledf_Control *self =
    _Workspace_Allocate_or_fatal_error( sizeof( *self ) );

  _RBTree_Initialize_empty( &self->ready,
			    &_Scheduler_EDF_RBTree_compare_function,
			    0
			  );
  _Chain_Initialize_empty( &self->scheduled );

  _Scheduler.information = self;
}

static void _Scheduler_globaledf_Allocate_processor(
  Thread_Control *scheduled,
  Thread_Control *victim
)
{
  printk(" 2 \n");
  Per_CPU_Control *cpu_of_scheduled = scheduled->cpu;
  Per_CPU_Control *cpu_of_victim = victim->cpu;
  Thread_Control *heir;

  scheduled->is_scheduled = true;
  victim->is_scheduled = false;

  if ( scheduled->is_executing ) {
    heir = cpu_of_scheduled->heir;
    cpu_of_scheduled->heir = scheduled;
  } else {
    heir = scheduled;
  }

  if ( heir != victim ) {
    heir->cpu = cpu_of_victim;
    cpu_of_victim->heir = heir;
    cpu_of_victim->dispatch_necessary = true;
  }
}

static void _Scheduler_globaledf_Move_from_scheduled_to_ready(
  RBTree_Control *ready_chain,
  Thread_Control *scheduled_to_ready
)
{
  printk("3 \n");
 Scheduler_globaledf_Control *self = _Scheduler_globaledf_Instance();
 RBTree_Node *node = (&self->node);
  _Chain_Extract_unprotected( &scheduled_to_ready->Object.Node );
 _RBTree_Insert( ready_chain, node );
 //scheduled_to_ready->queue_state = SCHEDULER_EDF_QUEUE_STATE_YES;
 //scheduled_to_ready->thread_location = THREAD_IN_READY_QUEUE;
 // RBTree Insert;//_Scheduler_simple_Insert_priority_lifo( ready_chain, scheduled_to_ready );
}

static void _Scheduler_globaledf_Move_from_ready_to_scheduled(
  Chain_Control *scheduled_chain,
  Thread_Control *ready_to_scheduled
)
{
  printk("4  \n");
   Scheduler_globaledf_perthread *sched_info =
    (Scheduler_globaledf_perthread*) ready_to_scheduled->scheduler_info;
   RBTree_Node *node = &(sched_info->Node);
 Scheduler_globaledf_Control *self = _Scheduler_globaledf_Instance();
 _RBTree_Extract(&self->ready, node );
 //ready_to_scheduled->thread_location = THREAD_IN_SCHEDULED_QUEUE;
  _Scheduler_simple_Insert_priority_fifo( scheduled_chain, ready_to_scheduled );
}

static void _Scheduler_globaledf_Insert(
  RBTree_Control *chain,
  Thread_Control *thread,
  RBTree_Node *node
)
{
   Scheduler_globaledf_perthread *sched_info =
    (Scheduler_globaledf_perthread*) thread->scheduler_info;
   /* RBTree_Node *node = &(sched_info->Node);
*/
   printk("5  \n");

   //  Scheduler_globaledf_Control *self = _Scheduler_globaledf_Instance();
  sched_info->thread_location = THREAD_IN_READY_QUEUE;
   _RBTree_Insert( chain, node);

  //    _Chain_Insert_ordered_unprotected( chain, &thread->Object.Node, order );
}

static void _Scheduler_globaledf_ChainInsert(
  Chain_Control *chain,
  Thread_Control *thread,
  Chain_Node_order order
)
{
  printk("6");
 Scheduler_globaledf_perthread *sched_info =
    (Scheduler_globaledf_perthread*) thread->scheduler_info;

  sched_info->thread_location = THREAD_IN_SCHEDULED_QUEUE;
  _Chain_Insert_ordered_unprotected( chain, &thread->Object.Node, order );
}



static void _Scheduler_globaledf_Enqueue_ordered(
  Thread_Control *thread,
  Chain_Node_order order,
 RBTree_Node *node
)
{
printk("7");
  Scheduler_globaledf_Control *self = _Scheduler_globaledf_Instance();

  /*
   * The scheduled chain has exactly processor count nodes after
   * initialization, thus the lowest priority scheduled thread exists.
   */
     Thread_Control *lowest_scheduled =
    (Thread_Control *) _Chain_Last( &self->scheduled );
     // if ( ( *order )( &thread->Object.Node, &lowest_scheduled->Object.Node ) ) {
    _Scheduler_globaledf_Allocate_processor( thread, lowest_scheduled );

    _Scheduler_globaledf_ChainInsert( &self->scheduled, thread, order );

    _Scheduler_globaledf_Move_from_scheduled_to_ready(
      &self->ready,
      lowest_scheduled
    );
    //  } else {
    //  _Scheduler_globaledf_Insert( &self->ready, thread, node);
    // }
}

void *_Scheduler_globaledf_Allocate( Thread_Control *the_thread)
{
printk("8");
 void *sched;
  Scheduler_globaledf_perthread *schinfo;

  sched = _Workspace_Allocate( sizeof(Scheduler_globaledf_perthread) );

  if ( sched ) {
    the_thread->scheduler_info = sched;
    schinfo = (Scheduler_globaledf_perthread *)(the_thread->scheduler_info);
    schinfo->thread = the_thread;
    schinfo->queue_state = SCHEDULER_EDF_QUEUE_STATE_NEVER_HAS_BEEN;
  }

  return sched;
}

void _Scheduler_globaledf_Enqueue_priority_lifo( Thread_Control *thread )
{
printk("9");
  /* _Scheduler_globaledf_Enqueue_ordered(
    thread,
    _Scheduler_simple_Insert_priority_lifo_order
    );*/

  _Scheduler_globaledf_Enqueue_priority_fifo(thread);
}

void _Scheduler_globaledf_Enqueue_priority_fifo( Thread_Control *thread )
{
printk("10 Enqueue priority");
 Scheduler_globaledf_perthread *sched_info =
    (Scheduler_globaledf_perthread*) thread->scheduler_info;
  RBTree_Node *node = &(sched_info->Node);

  _Scheduler_globaledf_Enqueue_ordered(
    thread,
    _Scheduler_simple_Insert_priority_fifo_order,
    node
  );
}

void _Scheduler_globaledf_Extract( Thread_Control *thread )
{
printk("11");
  Scheduler_globaledf_Control *self = _Scheduler_globaledf_Instance();

  _Chain_Extract_unprotected( &thread->Object.Node );

  if ( thread->is_scheduled ) {
    RBTree_Node *first = _RBTree_First(&self->ready, RBT_LEFT);
    Scheduler_globaledf_perthread *sched_info =  _RBTree_Container_of(first, Scheduler_globaledf_perthread, Node);

    Thread_Control *highest_ready = sched_info->thread;
      //  (Thread_Control *) _Chain_First( &self->ready );

    _Scheduler_globaledf_Allocate_processor( highest_ready, thread );

    _Scheduler_globaledf_Move_from_ready_to_scheduled(
      &self->scheduled,
      highest_ready
    );
  }
}

void _Scheduler_globaledf_Yield( Thread_Control *thread )
{
printk("12");
  ISR_Level level;

  _ISR_Disable( level );

  _Scheduler_globaledf_Extract( thread );
  _Scheduler_globaledf_Enqueue_priority_fifo( thread );

  _ISR_Enable( level );
}

void _Scheduler_globaledf_Schedule( void )
{
  /* Nothing to do */
}

void _Scheduler_globaledf_Start_idle(
  Thread_Control *thread,
  Per_CPU_Control *cpu
)
{
  printk("Start Idle \n");
  Scheduler_globaledf_Control *self = _Scheduler_globaledf_Instance();

  thread->is_scheduled = true;
  thread->cpu = cpu;
  _Chain_Append_unprotected( &self->scheduled, &thread->Object.Node );
}
  
