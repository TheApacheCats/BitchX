/*
 * funny.h: header for funny.c
 *
 * written by michael sandrof
 *
 * copyright(c) 1990 
 *
 * see the copyright file, or do a help ircii copyright 
 *
 * @(#)$Id$
 */
#ifndef FUNNY_H_
#define FUNNY_H_

#define FUNNY_PUBLIC 1
#define FUNNY_PRIVATE 2
#define FUNNY_TOPIC  4
#define FUNNY_WIDE   8
#define FUNNY_USERS  16
#define FUNNY_NAME   32

	void	set_funny_flags (int, int, int);
	void	funny_set_ignore_mode (void);
	int	funny_is_ignore_channel (void);
	void	funny_set_ignore_channel (char *);
	void	funny_match (char *);
	void	funny_print_widelist (void);
	void	funny_list (char *, char **);
	void	funny_mode (char *, char **);
	void	funny_namreply (char *, char **);

#endif /* FUNNY_H_ */
