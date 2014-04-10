/*******************************************************************************
	cbuf.inl: Onset detection threshold calculation.

	Copyright (C) 2014 Paul Barker, Loughborough University

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

#ifndef __TUNA_CBUF_INL_INCLUDED__
#define __TUNA_CBUF_INL_INCLUDED__

#if defined(ENABLE_INLINE) || defined(__TUNA_CBUF_C__)

#include <assert.h>

#include "types.h"

struct cbuf {
	uint index;
	uint len;

	env_t data[];
};

TUNA_INLINE env_t cbuf_rotate(struct cbuf *c, env_t s)
{
	assert(c);

	env_t tmp;

	tmp = c->data[c->index];
	c->data[c->index] = s;
	if (++c->index == c->len)
		c->index = 0;

	return tmp;
}

#if defined(ENABLE_INLINE) && defined(__TUNA_CBUF_C__)
TUNA_EXTERN_INLINE env_t cbuf_rotate(struct cbuf *c, env_t s);
#endif /* ENABLE_INLINE && __TUNA_CBUF_C__ */
#endif /* ENABLE_INLINE || __TUNA_CBUF_C__ */
#endif /* !__TUNA_CBUF_INL_INCLUDED__ */
