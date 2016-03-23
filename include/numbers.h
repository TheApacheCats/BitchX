/*
 * numbers.h: header for numbers.c
 *
 * written by michael sandrof
 *
 * copyright(c) 1990 
 *
 * see the copyright file, or do a help ircii copyright 
 *
 * @(#)$Id$
 */

#ifndef __numbers_h_
#define __numbers_h_

char *numeric_banner(void);
void display_msg(char *, char **);
void numbered_command(char *, int, char **);
int check_sync(int, char *, char *, char *, char *, ChannelList *);
		
#endif /* __numbers_h_ */
