/*******************************************************************************
	log.h: Generic logging support.

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

#ifndef __TUNA_LOG_H_INCLUDED__
#define __TUNA_LOG_H_INCLUDED__

#include "compiler.h"

/**
 * \file <tuna/log.h>
 *
 * Logging subsystem.
 */

/**
 * Initilize logging subsystem.
 *
 * \param output_file Path and filename to write log messages to. This file will
 * be created if it doesn't exist, otherwise the output will be appended to the
 * file.
 *
 * \param app_name Name for the current application. Enables the source of log
 * messages to be determined if multiple programs write to the same log file.
 *
 * \returns 0 on success, <0 on failure.
 */
int log_init(const char *output_file, const char *app_name);

/**
 * Shutdown the logging subsystem.
 */
void log_exit();

/**
 * Print output to a log file.
 *
 * \param level Severity level. If this is LOG_FATAL, the call aborts the
 * application after the log message is written.
 *
 * \param s printf-style format string, followed by arguments.
 */
int log_printf(int level, const char *s, ...);

/**
 * Equivalent to log_printf(LOG_MESSAGE, s, ...).
 */
int msg(const char *s, ...);

/**
 * Equivalent to log_printf(LOG_WARNING, s, ...).
 */
int warn(const char *s, ...);

/**
 * Equivalent to log_printf(LOG_ERROR, s, ...).
 */
int error(const char *s, ...);

/**
 * Equivalent to log_printf(LOG_FATAL, s, ...).
 *
 * This function terminates the application and so does not return.
 */
void __noreturn fatal(const char *s, ...);

/**
 * Sync the log output to disk.
 */
void log_sync();

/**
 * Valid log levels in decreasing order of severity.
 */
enum __log_levels {
	/**
	 * Fatal errors which cannot be recovered from and must cause immediate
	 * termination of the program.
	 */
	LOG_FATAL,

	/**
	 * Errors which may be recovered from.
	 */
	LOG_ERROR,

	/**
	 * Warnings.
	 */
	LOG_WARNING,

	/**
	 * General notice messages.
	 */
	LOG_MESSAGE,

	/**
	 * Internal marker value, not valid for use.
	 */
	LOG_MAX_LEVEL
};

#endif /* !__TUNA_LOG_H_INCLUDED__ */
