/*******************************************************************************
	cdeque.c: Deque with known maximum length, stored in a circular buffer.

	Copyright (C) 2013, 2014 Paul Barker, Loughborough University

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
#include <malloc.h>

#include "cdeque.h"
#include "compiler.h"
#include "log.h"
#include "types.h"

struct cdeque {
	/* Left index, points to the leftmost element in the deque. */
	uint left;

	/* Right index, points ONE ELEMENT PAST the rightmost element in the
	 * deque. Thus we can distinguish between an empty deque (left == right)
	 * and a full deque (left == right + 1).
	 */
	uint right;

	uint len;

	sample_t data[];
};

struct cdeque * cdeque_init(uint len)
{
	/* We need a spare value to use as a separator between the rightmost and
	 * the leftmost values in the deque.
	 */
	len++;

	struct cdeque * c = malloc(sizeof(struct cdeque) + len * sizeof(sample_t));
	if (!c) {
		error("cdeque_init: Failed to allocate memory");
		return NULL;
	}

	c->left = c->right = 0;
	c->len = len;

	return c;
}

void cdeque_reset(struct cdeque * c)
{
	assert(c);

	c->left = c->right = 0;
}

void cdeque_exit(struct cdeque * c)
{
	assert(c);

	free(c);
}

sample_t cdeque_popleft(struct cdeque * c)
{
	assert(c);

	if (c->left != c->right) {
		sample_t v = c->data[c->left];
		c->left = (c->left + 1) % c->len;

		return v;
	} else {
		/* TODO: Can we return a better error value. */
		return -1;
	}
}

sample_t cdeque_popright(struct cdeque * c)
{
	assert(c);

	if (c->left != c->right) {
		c->right = (c->right + c->len - 1) % c->len;

		return c->data[c->right];
	} else {
		/* TODO: Can we return a better error value. */
		return -1;
	}
}

void cdeque_pushleft(struct cdeque * c, sample_t v)
{
	assert(c);

	c->left = (c->left + c->len - 1) % c->len;

	c->data[c->left] = v;

	/* Check if the dequeue is full. */
	if (c->left == c->right)
		c->right = (c->right + c->len - 1) % c->len;
}

void cdeque_pushright(struct cdeque * c, sample_t v)
{
	assert(c);

	c->data[c->right] = v;
	c->right = (c->right + 1) % c->len;

	/* Check if the dequeue is full. */
	if (c->left == c->right)
		c->left = (c->left + 1) % c->len;
}

sample_t cdeque_left(struct cdeque * c)
{
	assert(c);

	if (c->left != c->right)
		return c->data[c->left];
	else
		/* TODO: Can we return a better error value. */
		return -1;
}

sample_t cdeque_right(struct cdeque * c)
{
	assert(c);

	if (c->left != c->right) {
		uint off = (c->right + c->len - 1) % c->len;

		return c->data[off];
	} else {
		/* TODO: Can we return a better error value. */
		return -1;
	}
}

uint cdeque_len(struct cdeque * c)
{
	assert(c);

	return (c->len + c->right - c->left) % c->len;
}
