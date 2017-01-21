/*
 * flood.h: header file for flood.c
 *
 * @(#)$Id$
 */

#ifndef __flood_h_
#define __flood_h_

enum flood_type {
	MSG_FLOOD,
	PUBLIC_FLOOD,
	NOTICE_FLOOD,
	WALL_FLOOD,
	WALLOP_FLOOD,
	CTCP_FLOOD,
	INVITE_FLOOD,
	CDCC_FLOOD,
	CTCP_ACTION_FLOOD,
	NICK_FLOOD,
	DEOP_FLOOD,
	KICK_FLOOD,
	JOIN_FLOOD
};

#define FLOOD_FLAG(t) (1U << (t))

int BX_check_flooding(char *nick, enum flood_type type, char *line, char *channel);
int BX_is_other_flood(ChannelList *channel, NickList *nick, enum flood_type type, int *t_flood);
int BX_flood_prot(char *nick, char *userhost, enum flood_type flood_type, int ignoretime, char *channel);
void clean_flood_list(void);

#include "hash.h"
#define FLOOD_HASHSIZE 31
extern HashEntry no_flood_list[FLOOD_HASHSIZE];

#endif /* __flood_h_ */
