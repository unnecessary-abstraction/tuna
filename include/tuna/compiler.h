/*******************************************************************************
	compiler.h: Define a few useful macros for gcc.

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

#ifndef __TUNA_COMPILER_H_INCLUDED__
#define __TUNA_COMPILER_H_INCLUDED__

#include <sys/cdefs.h>
#include <stddef.h>
#include <stdint.h>

#ifndef __noreturn
#define __noreturn __attribute__ ((noreturn))
#endif

#ifndef __unused
#define __unused (void)
#endif

/* container_of macro taken from Linux Kernel. */
#define container_of(ptr, type, member) ({				\
                const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
                (type *)( (char *)__mptr - offsetof(type,member) );	\
	})

#define ptr_offset(ptr, d) ({						\
		char * __cptr = (char *)(ptr);				\
		__cptr + d;						\
	})

#define ptr_truncate(ptr, round) ({					\
		uintptr_t __iptr = (uintptr_t)(ptr);			\
		uintptr_t __ir = (uintptr_t)(round);			\
		(void *)(__iptr & ~(__ir-1));				\
	})

#endif /* !__TUNA_COMPILER_H_INCLUDED__ */