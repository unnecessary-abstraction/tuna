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

#include <malloc.h>
#include <pthread.h>

#include "compiler.h"
#include "log.h"
#include "uara.h"

/*******************************************************************************
	Private declarations
*******************************************************************************/

#define BUFQ_NULL	0
#define BUFQ_BUFFER	1
#define BUFQ_START	2
#define BUFQ_RESYNC	3

struct bufq_entry {
	struct bufq_entry *		next;
	uint				event;

	union {
		struct {
			void *		buf;
			uint		count;
		};
		struct timespec		ts;
	};
};

struct bufq {
	struct consumer		consumer;

	struct consumer *	target;


	uint			sample_rate;
	int			exit;
	pthread_t		thread;

	/* The queue: push at tail, pop from head. */
	struct bufq_entry *	head;
	struct bufq_entry *	tail;

	/* Free entries: push at head, pop at head. */
	struct bufq_entry *	freestack;

	/* This structure needs to be accessed in a thread safe manner. */
	pthread_mutex_t		mutex;

	/* We need to be able to alert the consumer thread that data is available. */
	pthread_cond_t		cond;
};

/*******************************************************************************
	Private functions
*******************************************************************************/

static int enqueue_buffer(struct bufq * b, uint event, sample_t * buf, uint count)
{
	struct bufq_entry * e;

	pthread_mutex_lock(&b->mutex);

	if (b->freestack) {
		e = b->freestack;
		b->freestack = e->next;
	} else {
		e = (struct bufq_entry *)malloc(sizeof(struct bufq_entry));
		if (!e) {
			error("bufq: Failed to allocate memory");
			return -1;
		}
	}

	e->next = NULL;
	e->event = event;
	e->buf = buf;
	e->count = count;

	b->tail->next = e;
	b->tail = e;

	/* Signal that there is data in the queue. */
	pthread_cond_signal(&b->cond);

	pthread_mutex_unlock(&b->mutex);

	return 0;
}

static int enqueue_timespec(struct bufq * b, uint event, struct timespec * ts)
{
	struct bufq_entry * e;

	pthread_mutex_lock(&b->mutex);

	if (b->freestack) {
		e = b->freestack;
		b->freestack = e->next;
	} else {
		e = (struct bufq_entry *)malloc(sizeof(struct bufq_entry));
		if (!e) {
			error("bufq: Failed to allocate memory");
			return -1;
		}
	}

	e->next = NULL;
	e->event = event;
	e->ts = *ts;

	b->tail->next = e;
	b->tail = e;

	/* Signal that there is data in the queue. */
	pthread_cond_signal(&b->cond);

	pthread_mutex_unlock(&b->mutex);

	return 0;
}

static struct bufq_entry * dequeue(struct bufq * b)
{
	struct bufq_entry * e;

	pthread_mutex_lock(&b->mutex);

	e = b->head;
	if (!e) {
		/* No data available - wait until data is added to the queue. */
		pthread_cond_wait(&b->cond, &b->mutex);

		/* Maybe we were signalled to exit. */
		if (b->exit)
			pthread_exit(NULL);

		/* Now we should have data or have been signalled to exit. */
		e = b->head;
		if (!e) {
			error("bufq: Signal raised incorrectly");
			return NULL;
		}
	}

	b->head = e->next;

	pthread_mutex_unlock(&b->mutex);

	return e;
}

static void free_entry(struct bufq * b, struct bufq_entry * e)
{
	pthread_mutex_lock(&b->mutex);

	e->next = b->freestack;
	b->freestack = e;

	pthread_mutex_unlock(&b->mutex);
}

static void * consumer_thread(void * param)
{
	struct bufq * b = (struct bufq *)param;
	struct bufq_entry * e;
	uint err_count = 0;

	while (1) {
		/* Check for termination signal. */
		if (b->exit)
			pthread_exit(NULL);

		e = dequeue(b);

		if (!e) {
			err_count++;
			if (err_count > 5) {
				error("bufq: Too much consecutive missing data");
				pthread_exit(NULL);
			} else {
				error("bufq: Ignoring missing data");
				continue;
			}
		} else {
			err_count = 0;
		}

		switch (e->event) {
			case BUFQ_BUFFER:
				b->target->write(b->target, e->buf, e->count);
				break;

			case BUFQ_START:
				b->target->start(b->target, b->sample_rate, &e->ts);
				break;

			case BUFQ_RESYNC:
				b->target->resync(b->target, &e->ts);
				break;

			default:
				error("bufq: Ignoring unknown event");
		}

		free_entry(b, e);
	}
}

void bufq_exit(struct consumer * consumer)
{
	struct bufq * b = container_of(consumer, struct bufq, consumer);

	/* Signal consumer thread to terminate. */
	b->exit = 1;
	pthread_cond_signal(&b->cond);
	pthread_join(b->thread, NULL);

	pthread_cond_destroy(&b->cond);
	pthread_mutex_destroy(&b->mutex);
	free(b);
}

int bufq_write(struct consumer * consumer, sample_t * buf, uint count)
{
	struct bufq * b = container_of(consumer, struct bufq, consumer);

	return enqueue_buffer(b, BUFQ_BUFFER, buf, count);
}

int bufq_start(struct consumer * consumer, uint sample_rate, struct timespec * ts)
{
	struct bufq * b = container_of(consumer, struct bufq, consumer);

	b->sample_rate = sample_rate;
	return enqueue_timespec(b, BUFQ_START, ts);
}

int bufq_resync(struct consumer * consumer, struct timespec * ts)
{
	struct bufq * b = container_of(consumer, struct bufq, consumer);

	return enqueue_timespec(b, BUFQ_RESYNC, ts);
}

/*******************************************************************************
	Public functions
*******************************************************************************/

struct consumer * bufq_init(struct consumer * target)
{
	int r;
	struct bufq * b = (struct bufq *)malloc(sizeof(struct bufq));
	if (!b) {
		error("bufq_init: Failed to allocate memory");
		goto err_malloc;
	}

	b->target = target;
	b->head = NULL;
	b->tail = NULL;
	b->freestack = NULL;
	b->exit = 0;
	b->sample_rate = 0;

	r = pthread_mutex_init(&b->mutex, NULL);
	if (r != 0) {
		error("bufq: Failed to create mutex");
		goto err_mutex;
	}

	r = pthread_cond_init(&b->cond, NULL);
	if (r != 0) {
		error("bufq: Failed to create condition variable");
		goto err_cond;
	}

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
	free(b);
err_malloc:
	return NULL;
}
