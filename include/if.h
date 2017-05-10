/*
 * if.h: header for if.c
 *
 * copyright(c) 1994 matthew green
 *
 * See the copyright file, or do a help ircii copyright 
 *
 * @(#)$Id$
 */
#ifndef IF_H_
#define IF_H_

	void	ifcmd (char *, char *, char *, char *);
	void	whilecmd (char *, char *, char *, char *);
	void	foreach_handler (char *, char *, char *);
	void	foreach (char *, char *, char *, char *);
	void	fe (char *, char *, char *, char *);
	void	forcmd (char *, char *, char *, char *);
	void	fec (char *, char *, char *, char *);
	void	docmd (char *, char *, char *, char *);
	void	switchcmd (char *, char *, char *, char *);
extern	char *  next_expr       	(char **, char);
extern	char *  next_expr_failok        (char **, char);
  		
#endif /* IF_H_ */
