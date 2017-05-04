/*
 * hash.h: header file for hash.c
 *
 * Written by Scott H Kilau
 *
 * CopyRight(c) 1997
 *
 * See the COPYRIGHT file, or do a HELP IRCII COPYRIGHT
 *
 * @(#)$Id$
 */
#ifndef HASH_H_
#define HASH_H_

#define NICKLIST_HASHSIZE 79
#define WHOWASLIST_HASHSIZE 271

#ifndef REMOVE_FROM_LIST
# define REMOVE_FROM_LIST 1
#endif

/* hashentry: structure for all hash lists we make. 
 * quite generic, but powerful.
 */
typedef	struct hashentry
{
	void *list;		/* our linked list, generic void * */
	unsigned short hits;	/* how many hits this spot has gotten */
	unsigned short links;	/* how many links we have at this spot */
}	HashEntry;

#endif
