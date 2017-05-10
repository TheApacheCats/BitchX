/*
 * status.h: header for status.c
 *
 * Written By Michael Sandrof
 *
 * Copyright(c) 1990 
 *
 * See the COPYRIGHT file, or do a HELP IRCII COPYRIGHT 
 *
 * @(#)$Id$
 */
#ifndef STATUS_H_
#define STATUS_H_

	void	make_status (Window *);
	void	set_alarm (Window *, char *, int);
	char	*BX_update_clock (int);
	void	reset_clock (Window *, char *, int);
	void	BX_build_status (Window *, char *, int);
	int	BX_status_update (int);

#define GET_TIME 1
#define RESET_TIME 2

#endif /* STATUS_H_ */
