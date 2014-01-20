/*******************************************************************************
	cdeque.h: Deque with known maximum length, stored in a circular buffer.

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

#ifndef __TUNA_CDEQUE_H_INCLUDED__
#define __TUNA_CDEQUE_H_INCLUDED__

#include "types.h"

struct cdeque * cdeque_init(uint len);
void cdeque_reset(struct cdeque * c);
void cdeque_exit(struct cdeque * c);
sample_t cdeque_popleft(struct cdeque * c);
sample_t cdeque_popright(struct cdeque * c);
void cdeque_pushleft(struct cdeque * c, sample_t v);
void cdeque_pushright(struct cdeque * c, sample_t v);
sample_t cdeque_left(struct cdeque * c);
sample_t cdeque_right(struct cdeque * c);
uint cdeque_len(struct cdeque * c);

#endif /* !__TUNA_CDEQUE_H_INCLUDED__ */
