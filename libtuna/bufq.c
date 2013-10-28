/*******************************************************************************
	bufq.c: Buffer queue to decouple input from output.

	Copyright (C) 2013 Paul Barker, Loughborough University
	
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*******************************************************************************/

#include <assert.h>
#include <errno.h>
#include <malloc.h>
#include <pthread.h>

#include "buffer.h"
#include "compiler.h"
#include "consumer.h"
#include "list.h"
#include "log.h"

/*******************************************************************************
	Private declarations
*******************************************************************************/

#define BUFQ_NULL	0
#define BUFQ_WRITE	1
#define BUFQ_START	2
#define BUFQ_RESYNC	3

struct bufq_entry {
	uint				event;

	union {
		struct {
			void *		buf;
			uint		count;
		};
		struct timespec		ts;
	};

	struct list_entry		l;
};

struct bufq {
	struct consumer		consumer;

	struct consumer *	target;


	uint			sample_rate;
	volatile int		exit;
	pthread_t		thread;

	/* Rather than messing with the (void *) return type of a thread, we
	 * just store the consumer thread's exit value here. This will be zero
	 * if the thread is still running, 1 for clean termination and <0 on
	 * error.
	 */
	int			thread_exit_status;

	/* The queue: push at tail, pop from head. */
	struct list		queue;

	/* Free entries: push at head, pop at head. */
	struct list		freestack;

	/* This structure needs to be accessed in a thread safe manner. */
	pthread_mutex_t		mutex;

	/* We need to be able to alert the consumer thread that data is available. */
	pthread_cond_t		cond;
};

/*******************************************************************************
	Private functions
*******************************************************************************/

static struct bufq_entry * alloc_entry(struct bufq * b)
{
	assert(b);

	struct bufq_entry * e;
	struct list_entry * l;

	pthread_mutex_lock(&b->mutex);

	l = list_pop(&b->freestack);
	if (l)
		e = container_of(l, struct bufq_entry, l);
	else {
		e = (struct bufq_entry *)malloc(sizeof(struct bufq_entry));
	}

	pthread_mutex_unlock(&b->mutex);

	return e;
}

static void free_entry(struct bufq * b, struct bufq_entry * e)
{
	assert(b);
	assert(e);

	pthread_mutex_lock(&b->mutex);

	list_push(&b->freestack, &e->l);

	pthread_mutex_unlock(&b->mutex);
}

static int enqueue(struct bufq * b, struct bufq_entry * e)
{
	assert(b);
	assert(e);

	pthread_mutex_lock(&b->mutex);

	list_enqueue(&b->queue, &e->l);

	/* Signal that there is data in the queue. */
	pthread_cond_signal(&b->cond);

	pthread_mutex_unlock(&b->mutex);

	return 0;
}

static struct bufq_entry * dequeue(struct bufq * b)
{
	assert(b);

	struct bufq_entry * e;
	struct list_entry * l;

	pthread_mutex_lock(&b->mutex);

	l = list_dequeue(&b->queue);
	if (!l) {
		/* No data available - wait until data is added to the queue. */
		pthread_cond_wait(&b->cond, &b->mutex);

		/* Now we should have data or have been signalled to exit. */
		if (b->exit) {
			b->thread_exit_status = 1;
			pthread_exit(NULL);
		}

		l = list_dequeue(&b->queue);
		if (!l) {
			error("bufq: Signal raised incorrectly");
			return NULL;
		}
	}

	pthread_mutex_unlock(&b->mutex);

	e = container_of(l, struct bufq_entry, l);

	return e;
}

static int enqueue_buffer(struct bufq * b, uint event, sample_t * buf, uint count)
{
	assert(b);
	assert(buf);

	struct bufq_entry * e;

	e = alloc_entry(b);
	if (!e) {
		error("bufq: Failed to allocate memory");
		return -ENOMEM;
	}

	e->event = event;
	e->buf = buf;
	e->count = count;

	return enqueue(b, e);
}

static int enqueue_timespec(struct bufq * b, uint event, struct timespec * ts)
{
	assert(b);
	assert(ts);

	struct bufq_entry * e;

	e = alloc_entry(b);
	if (!e) {
		error("bufq: Failed to allocate memory");
		return -ENOMEM;
	}

	e->event = event;
	e->ts = *ts;

	return enqueue(b, e);
}

static void * consumer_thread(void * param)
{
	int r;
	struct bufq * b = (struct bufq *)param;
	struct bufq_entry * e;
	uint err_count = 0;

	while (1) {
		/* Check for termination signal. */
		if (b->exit) {
			b->thread_exit_status = 1;
			return NULL;
		}

		e = dequeue(b);

		if (!e) {
			err_count++;
			if (err_count > 5) {
				error("bufq: Too much consecutive missing data");
				b->thread_exit_status = -EIO;
				return NULL;
			} else {
				error("bufq: Ignoring missing data");
				continue;
			}
		} else {
			err_count = 0;
		}

		switch (e->event) {
			case BUFQ_WRITE:
				r = b->target->write(b->target, e->buf, e->count);
				if (r < 0) {
					error("bufq: target->write failed");
					b->thread_exit_status = r;
					return NULL;
				}
				buffer_release(e->buf);
				break;

			case BUFQ_START:
				r = b->target->start(b->target, b->sample_rate, &e->ts);
				if (r < 0) {
					error("bufq: target->start failed");
					b->thread_exit_status = r;
					return NULL;
				}
				break;

			case BUFQ_RESYNC:
				r = b->target->resync(b->target, &e->ts);
				if (r < 0) {
					error("bufq: target->resync failed");
					b->thread_exit_status = r;
					return NULL;
				}
				break;

			default:
				error("bufq: Ignoring unknown event");
		}

		free_entry(b, e);
	}
}

void bufq_exit(struct consumer * consumer)
{
	assert(consumer);

	struct bufq * b = container_of(consumer, struct bufq, consumer);

	/* Signal consumer thread to terminate. */
	b->exit = 1;
	pthread_cond_signal(&b->cond);
	pthread_join(b->thread, NULL);

	list_exit(&b->freestack);
	list_exit(&b->queue);
	pthread_cond_destroy(&b->cond);
	pthread_mutex_destroy(&b->mutex);
	free(b);
}

int bufq_write(struct consumer * consumer, sample_t * buf, uint count)
{
	assert(consumer);
	assert(buf);

	struct bufq * b = container_of(consumer, struct bufq, consumer);

	buffer_addref(buf);
	return enqueue_buffer(b, BUFQ_WRITE, buf, count);
}

int bufq_start(struct consumer * consumer, uint sample_rate, struct timespec * ts)
{
	assert(consumer);
	assert(ts);

	struct bufq * b = container_of(consumer, struct bufq, consumer);

	b->sample_rate = sample_rate;
	return enqueue_timespec(b, BUFQ_START, ts);
}

int bufq_resync(struct consumer * consumer, struct timespec * ts)
{
	assert(consumer);
	assert(ts);

	struct bufq * b = container_of(consumer, struct bufq, consumer);

	return enqueue_timespec(b, BUFQ_RESYNC, ts);
}

/*******************************************************************************
	Public functions
*******************************************************************************/

struct consumer * bufq_init(struct consumer * target)
{
	assert(target);

	int r;
	pthread_mutexattr_t attr;
	struct bufq * b = (struct bufq *)malloc(sizeof(struct bufq));
	if (!b) {
		error("bufq: Failed to allocate memory");
		goto err_malloc;
	}

	b->target = target;
	b->exit = 0;
	b->sample_rate = 0;

	list_init(&b->queue);
	list_init(&b->freestack);

	/* Create a recursive mutex. */
	r = pthread_mutexattr_init(&attr);
	if (r != 0) {
		error("bufq: Failed to create mutexattr");
		goto err_mutexattr;
	}

	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

	r = pthread_mutex_init(&b->mutex, &attr);
	pthread_mutexattr_destroy(&attr);
	if (r != 0) {
		error("bufq: Failed to create mutex");
		goto err_mutex;
	}

	r = pthread_cond_init(&b->cond, NULL);
	if (r != 0) {
		error("bufq: Failed to create condition variable");
		goto err_cond;
	}

	b->thread_exit_status = 0;
	r = pthread_create(&b->thread, NULL, consumer_thread, b);
	if (r != 0) {
		error("bufq: Failed to start consumer thread");
		goto err_thread;
	}

	/* Setup consumer and return. */
	b->consumer.write = bufq_write;
	b->consumer.start = bufq_start;
	b->consumer.resync = bufq_resync;
	b->consumer.exit = bufq_exit;

	return &b->consumer;

	/* Error cleanup */
err_thread:
	pthread_cond_destroy(&b->cond);
err_cond:
	pthread_mutex_destroy(&b->mutex);
err_mutex:
err_mutexattr:
	free(b);
err_malloc:
	return NULL;
}
