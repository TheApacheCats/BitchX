/*
 * log.h: header for log.c 
 *
 * Written By Michael Sandrof
 *
 * Copyright(c) 1990 
 *
 * See the COPYRIGHT file, or do a HELP IRCII COPYRIGHT 
 *
 * @(#)$Id$
 */
#ifndef LOG_H_
#define LOG_H_

	void	do_log (int, char *, FILE **);
	void	logger (Window *, char *, int);
	void	set_log_file (Window *, char *, int);
	void	BX_add_to_log (FILE *, time_t, const char *, int mangler);


#endif /* LOG_H_ */
