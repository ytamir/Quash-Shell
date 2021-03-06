/*
 * FILE		: pid_queue.c
 *
 * BRIEF	: Generates functions for operating on pid_queue_t data
 * 		  structure
 */

#include "job_queue.h"

/*
 *Use the prototype macro to place all the pid_queue related functions here
 */
IMPLEMENT_DEQUE (job_queue, jobtype);
