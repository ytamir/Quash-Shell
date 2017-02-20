/*
 * FILE		: pid_queue.h
 *
 * BRIEF	: Generates declarations for deque data structure to store
 * 		  process ids
 */

#ifndef __PID_QUEUE_H__
#define __PID_QUEUE_H__

#include "deque.h"

/*
 * Declare the queue data structure
 */
IMPLEMENT_DEQUE_STRUCT (pid_queue, pid_t);

/*
 * Declare a queue for storing process ids (i.e. integers)
 */
PROTOTYPE_DEQUE (pid_queue, pid_t);

#endif /* __PID_QUEUE_H__ */
