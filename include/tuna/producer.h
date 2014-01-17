/*******************************************************************************
	producer.h: Producer type.

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

#ifndef __TUNA_PRODUCER_H_INCLUDED__
#define __TUNA_PRODUCER_H_INCLUDED__

struct producer;

typedef int (*producer_run_fn)(struct producer * producer);
typedef int (*producer_stop_fn)(struct producer * producer, int condition);
typedef void (*producer_exit_fn)(struct producer * producer);

struct producer * producer_new();
void producer_exit(struct producer * producer);

void producer_set_module(struct producer * producer, producer_run_fn run,
		producer_stop_fn stop, producer_exit_fn exit, void * data);
void * producer_get_data(struct producer * producer);

int producer_run(struct producer * producer);
int producer_stop(struct producer * producer, int condition);

#endif /* !__TUNA_PRODUCER_H_INCLUDED__ */
