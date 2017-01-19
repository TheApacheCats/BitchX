/*
 * flood.h: header file for flood.c
 *
 * @(#)$Id$
 */

#ifndef __flood_h_
#define __flood_h_

enum flood_type {
	MSG_FLOOD = 0x0001,
	PUBLIC_FLOOD = 0x0002,
	NOTICE_FLOOD = 0x0004,
	WALL_FLOOD = 0x0008,
	WALLOP_FLOOD = 0x0010,
	CTCP_FLOOD = 0x0020,
	INVITE_FLOOD = 0x0040,
	CDCC_FLOOD = 0x0080,
	CTCP_ACTION_FLOOD = 0x0100,
	NICK_FLOOD = 0x0200,
	DEOP_FLOOD = 0x0400,
	KICK_FLOOD = 0x0800,
	JOIN_FLOOD = 0x1000
};

#define FLOOD_FLAG(t) ((unsigned)(t))

int BX_check_flooding(char *nick, enum flood_type type, char *line, char *channel);
int BX_is_other_flood(ChannelList *channel, NickList *nick, enum flood_type type, int *t_flood);
int BX_flood_prot(char *nick, char *userhost, enum flood_type flood_type, int ignoretime, char *channel);
void clean_flood_list(void);

#include "hash.h"
#define FLOOD_HASHSIZE 31
extern HashEntry no_flood_list[FLOOD_HASHSIZE];

#endif /* __flood_h_ */
