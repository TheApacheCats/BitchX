/*
 * irc_std.h: header to define things used in all the programs ircii
 * comes with
 *
 * hacked together from various other files by matthew green
 * copyright(c) 1993 
 *
 * See the copyright file, or do a help ircii copyright 
 *
 * @(#)$Id$
 */
#ifndef IRC_STD_H_
#define IRC_STD_H_

#include "defs.h"

/*
 * Everybody needs these ANSI headers...
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <stddef.h>

/*
 * Everybody needs these POSIX headers...
 */
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <limits.h>
#include <sys/param.h>
#include <errno.h>

/*
 * Everybody needs these INET headers...
 */
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif

/*
 * Deal with brokenness in <time.h> and <sys/time.h>
 */
#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

/*
 * Deal with brokenness in <fcntl.h> and <sys/fcntl.h>
 */
#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#else
# ifdef HAVE_SYS_FCNTL_H
#  include <sys/fcntl.h>
# endif
#endif

/*
 * Deal with brokenness figuring out struct direct
 */
#if HAVE_DIRENT_H
# include <dirent.h>
# define NAMLEN(dirent) strlen((dirent)->d_name)
#else
# define dirent direct
# define NAMLEN(dirent) (dirent)->d_namlen
# if HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif
# if HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif
# if HAVE_NDIR_H
#  include <ndir.h>
# endif
#endif

/*
 * First try to figure out if we can use GNU CC special features...
 */
#ifndef __GNUC__
# define _printf_(x)
# define _noreturn_
#else
# if (__GNUC__ > 2) || ((__GNUC__ == 2) && (__GNUC_MINOR__ >= 7))
#  define _printf_(x) __attribute__ ((format (printf, x, x + 1)))
#  define _noreturn_  __attribute__ ((noreturn))
# else
#  define _printf_(x)
#  define _noreturn_
# endif
#endif

/*
 * Figure out how to make alloca work
 * I took this from the autoconf documentation
 */
#if defined(__GNUC__) && !defined(HAVE_ALLOCA_H)
# ifndef alloca
#  define alloca __builtin_alloca
# endif
#else
# if HAVE_ALLOCA_H
#  include <alloca.h>
# else
#  ifdef _AIX
 #pragma alloca
#  else
#   ifndef alloca
char *alloca();
#   endif
#  endif
# endif
#endif

# include <errno.h>
#ifndef ERRNO_DECLARED
extern	int	errno;
#endif

#ifndef HAVE_SOCKLEN_T
typedef int socklen_t;
#endif

#ifndef NBBY
# define NBBY		8			/* number of bits in a byte */
#endif /* NBBY */

#ifndef NFDBITS
# define NFDBITS	(sizeof(long) * NBBY)	/* bits per mask */
#endif /* NFDBITS */

#ifndef	FD_SETSIZE
#define FD_SETSIZE	256
#endif

#ifndef howmany
#define howmany(x, y)   (((x) + ((y) - 1)) / (y))
#endif

#include <limits.h>
   
#define SIGNAL_HANDLER(x) \
	RETSIGTYPE x (int unused)

typedef SIGNAL_HANDLER(sigfunc);
sigfunc *my_signal (int, sigfunc *, int);

#include <stdlib.h>
#define index strchr

#ifndef MAXPATHLEN
#ifndef PATHSIZE
#define PATHSIZE 1024
#endif
#define MAXPATHLEN  PATHSIZE
#endif

/*
 * Dont trust anyone else's NULLs.
 */
#ifdef NULL
#undef NULL
#endif
#define NULL (void *) 0

#ifndef HAVE_STRERROR
#ifndef SYS_ERRLIST_DECLARED
extern  char    *sys_errlist[];
#endif
#define strerror(x) (char *)sys_errlist[x]
#endif

#if !defined(HAVE_GETTIMEOFDAY) && defined(HAVE_SYS_TIME_H)
extern	int	gettimeofday(struct timeval *tv, struct timezone *tz);
#endif

#ifndef GETPGID_DECLARED
pid_t	getpgid (pid_t pid);
#endif

#ifndef KILLPG_DECLARED
int	killpg (int pgrp, int sig);
#endif

#ifndef GETPASS_DECLARED
char *	getpass (const char * prompt);
#endif

#define BUILT_IN_COMMAND(x) \
	void x (char *command, char *args, char *subargs, char *helparg)

#define BUILT_IN_FUNCTION(x) \
	char * x (char *fn, char *input)

#if defined(_AIX)
int getpeername (int s, struct sockaddr *, int *);
int getsockname (int s, struct sockaddr *, int *);
int socket (int, int, int);
int bind (int, struct sockaddr *, int);
int listen (int, int);
int accept (int, struct sockaddr *, int *);
int recv (int, void *, int, unsigned int);
int send (int, void *, int, unsigned int);
int gettimeofday (struct timeval *, struct timezone *);
int gethostname (char *, int);
int setsockopt (int, int, int, void *, int);
int setitimer (int, struct itimerval *, struct itimerval *);
int ioctl (int, int, ...);
#endif

#ifdef __EMX__
#define strcasecmp stricmp
#define strncasecmp strnicmp
#endif

/* We provide our own ltoa() rather than rely on the various non-standard ones
 * that some platforms provide. */
#define ltoa(a) my_ltoa(a)

/*
 * DCC specification requires exactly a 32 bit checksum.
 * Kind of lame, actually.
 */
#ifdef UNSIGNED_LONG32
  typedef		unsigned long		u_32int_t;
#else
# ifdef UNSIGNED_INT32
  typedef		unsigned int		u_32int_t;
# else
  typedef		unsigned long		u_32int_t;
# endif
#endif

#if !HAVE_DECL_SYS_SIGLIST && HAVE_DECL__SYS_SIGLIST
#define sys_siglist _sys_siglist
#endif

/* Used in compat.c */
#ifndef HAVE_TPARM
	char 	*tparm (const char *, ...);
#endif

#ifndef HAVE_STRTOUL
	unsigned long 	strtoul (const char *, char **, int);
#endif

#ifndef HAVE_SETENV
	char *	bsd_getenv (const char *);
	int	bsd_putenv (const char *);
	int	bsd_setenv (const char *, const char *, int);
	void	bsd_unsetenv (const char *);
#define setenv bsd_setenv
#endif

#ifndef HAVE_INET_ATON
	int	inet_aton (const char *, struct in_addr *);
#endif

#ifndef HAVE_STRLCPY
	size_t	strlcpy (char *, const char *, size_t);
#endif

#ifndef HAVE_STRLCAT
	size_t	strlcat (char *, const char *, size_t);
#endif

#ifndef HAVE_VSNPRINTF
	int	vsnprintf (char *, size_t, const char *, va_list);
#endif

#ifndef HAVE_SNPRINTF
	int	snprintf (char *, size_t, const char *, ...);
#endif

#ifndef HAVE_SETSID
	int	setsid (void);
#endif

#endif /* IRC_STD_H_ */
