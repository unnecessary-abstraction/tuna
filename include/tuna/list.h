/*******************************************************************************
	list.h: Basic list data structure.

	Copyright (C) 2006-2013 Paul Barker
	
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

#ifndef __TUNA_LIST_H_INCLUDED__
#define __TUNA_LIST_H_INCLUDED__

#include <assert.h>
#include <stdbool.h>

struct list_entry {
	struct list_entry * next;
	struct list_entry * prev;
};

/*
    A doubly linked list.

    We use a head and a tail to ensure that any valid node has valid next and
    prev pointers, thus simplifying the insertion and removal code.

    Note that the assertions used here are very paranoid.
*/
struct list {
	/* Head has prev=0. */
	struct list_entry head;

	/* Tail has next=0. */
	struct list_entry tail;
};

/*******************************************************************************
	Element conditionals: is_head, is_tail
*******************************************************************************/

static inline bool list_is_head(struct list_entry * e)
{
	assert(e);
	assert(e->prev);

	return !e->prev->prev;
}

static inline bool list_is_tail(struct list_entry * e)
{
	assert(e);
	assert(e->next);

	return !e->next->next;
}

/*******************************************************************************
	Element access: next, prev, head and tail
*******************************************************************************/

static inline struct list_entry * list_next(struct list_entry * e)
{
	assert(e);
	assert(e->next);

	if (list_is_tail(e))
		return NULL;

	return e->next;
}

static inline struct list_entry * list_prev(struct list_entry * e)
{
	assert(e);
	assert(e->prev);

	if (list_is_head(e))
		return NULL;

	return e->prev;
}

static inline struct list_entry * list_head(struct list * li)
{
	assert(li);
	return list_next(&li->head);
}

static inline struct list_entry * list_tail(struct list * li)
{
	assert(li);
	return list_prev(&li->tail);
}

/*******************************************************************************
	Whole list conditionals: is_empty
*******************************************************************************/

static inline bool list_is_empty(struct list * s)
{
	assert(s);

	return !list_head(s);
}

/*******************************************************************************
	List initialisation: init and exit.
*******************************************************************************/

static inline int list_init(struct list * li)
{
	assert(li);

	/* Setup the head. */
	li->head.next = &li->tail;
	li->head.prev = NULL;

	/* Setup the tail. */
	li->tail.next = NULL;
	li->tail.prev = &li->head;

	return 0;
}

static inline void list_exit(struct list * li)
{
	assert(li);
}

/*******************************************************************************
	List modification: insert_before, insert_after and remove
*******************************************************************************/

static inline void list_insert_after(struct list_entry * e, struct list_entry * new)
{
	assert(e);
	assert(e->next);
	assert(new);

	new->next = e->next;
	new->next->prev = new;

	e->next = new;
	new->prev = e;
}

static inline void list_insert_before(struct list_entry * e, struct list_entry * new)
{
	assert(e);
	assert(e->prev);
	assert(new);

	new->prev = e->prev;
	new->prev->next = new;

	e->prev = new;
	new->next = e;
}

static inline void list_remove(struct list_entry * e)
{
	assert(e);
	assert(e->next);
	assert(e->prev);

	e->prev->next = e->next;
	e->next->prev = e->prev;

	e->next = NULL;
	e->prev = NULL;
}

/*******************************************************************************
	Convenience functions: prepend and append
*******************************************************************************/

static inline void list_prepend(struct list * li, struct list_entry * e)
{
	assert(li);
	list_insert_after(&li->head, e);
}

static inline void list_append(struct list * li, struct list_entry * e)
{
	assert(li);
	list_insert_before(&li->tail, e);
}

/*******************************************************************************
	Stack functions: push and pop
*******************************************************************************/

static inline void list_push(struct list * li, struct list_entry * e)
{
	list_prepend(li, e);
}

static inline struct list_entry * list_pop(struct list * li)
{
	struct list_entry * e = list_head(li);
	list_remove(e);
	return e;
}

/*******************************************************************************
	Queue function: enqueue and dequeue
*******************************************************************************/

static inline void list_enqueue(struct list * li, struct list_entry * e)
{
	list_append(li, e);
}

static inline struct list_entry * list_dequeue(struct list * li)
{
	return list_pop(li);
}

#endif /* !__TUNA_LIST_H_INCLUDED__ */
