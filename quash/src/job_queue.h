/*
 * FILE		: pid_queue.h
 *
 * BRIEF	: Generates declarations for deque data structure to store
 * 		  process ids
 */

#ifndef __JOB_QUEUE_H__
#define __JOB_QUEUE_H__

#include "deque.h"
#include "pid_queue.h"

typedef struct Job{
    int id;
    pid_queue process_queue;
    char* cmd;
    pid_t pid;
} jobtype;

/*
 * Declare the queue data structure
 */
IMPLEMENT_DEQUE_STRUCT (job_queue, jobtype);

/*
 * Declare a queue for storing process ids (i.e. integers)
 */
PROTOTYPE_DEQUE (job_queue, jobtype);

#endif /* __JOB_QUEUE_H__ */
