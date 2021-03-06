/*
 * this code is Copyright Colten Edwards (c) 96
 */
 
#include "irc.h"
static char cvsrevision[] = "$Id$";
CVS_REVISION(banlist_c)
#include "struct.h"
#include "commands.h"
#include "list.h"
#include "hook.h"
#include "ignore.h"
#include "ircaux.h"
#include "output.h"
#include "screen.h"
#include "server.h"
#include "window.h"
#include "who.h"
#include "whowas.h"
#include "vars.h"
#include "userlist.h"
#include "misc.h"
#include "timer.h"
#include "hash2.h"
#include "cset.h"
#define MAIN_SOURCE
#include "modval.h"

#ifndef Server2_8hybrid
#define Server2_8hybrid 5
#endif

int defban = 2;

static char *mode_buf = NULL;
static int mode_len = 0;

static char *mode_str = NULL;
static char *user = NULL;
static int mode_str_len = 0;
static int push_len = 0;
static char plus_mode[20] = "\0";

void add_mode_buffer(char *buffer, int mode_str_len)
{
	malloc_strcat(&mode_buf, buffer);
	mode_len += push_len;
}

void flush_mode(ChannelList *chan)
{
	
	if (mode_buf)
		my_send_to_server(chan?chan->server:from_server, "%s", mode_buf);
	new_free(&mode_buf);
	mode_len = 0;
}

int delay_flush_all(void *arg, char *sub)
{
	char *channel, *serv_num, *args = (char *)arg;
	int ofs = from_server;
	char buffer[BIG_BUFFER_SIZE+1];
	
	channel = next_arg(args, &args);
	if ((serv_num = next_arg(args, &args)))
		from_server = atoi(serv_num);
	if (channel && *channel && mode_str && user)
	{
		sprintf(buffer, "MODE %s %s%s %s\r\n", channel, plus_mode, mode_str, user);
		push_len = strlen(buffer);
		add_mode_buffer(buffer, push_len);
		mode_str_len = 0;
		new_free(&mode_str);
		new_free(&user);
		memset(plus_mode, 0, sizeof(plus_mode));
		push_len = 0;
	}
	flush_mode(NULL);
	new_free(&arg);
	from_server = ofs;
	return 0;
}

void flush_mode_all(ChannelList *chan)
{
	char buffer[BIG_BUFFER_SIZE+1];
	
	if (mode_str && user)
	{
		sprintf(buffer, "MODE %s %s%s %s\r\n", chan->channel, plus_mode, mode_str, user);
		push_len = strlen(buffer);
		add_mode_buffer(buffer, push_len);
		mode_str_len = 0;
		new_free(&mode_str);
		new_free(&user);
		memset(plus_mode, 0, sizeof(plus_mode));
		push_len = 0;
	}
	flush_mode(chan);
}

void add_mode(ChannelList *chan, char *mode, int plus, char *nick, char *reason, int max_modes)
{
	char buffer[BIG_BUFFER_SIZE+1];
	
	if (mode_len >= (IRCD_BUFFER_SIZE-100))
	{
		flush_mode(chan);
		push_len = 0;
	}

	if (reason)
	{
		sprintf(buffer, "KICK %s %s :%s\r\n", chan->channel, nick, reason);
		push_len = strlen(buffer);
		add_mode_buffer(buffer, push_len);
	}
	else
	{
		mode_str_len++;
		strcat(plus_mode, plus ? "+" : "-");
		malloc_strcat(&mode_str, mode);
		m_s3cat(&user, space, nick);
		if (mode_str_len >= max_modes)
		{
			sprintf(buffer, "MODE %s %s%s %s\r\n", chan->channel, plus_mode, mode_str, user);
			push_len = strlen(buffer);
			add_mode_buffer(buffer, push_len);
			new_free(&mode_str);
			new_free(&user);
			memset(plus_mode, 0, sizeof(plus_mode));
			mode_str_len = push_len = 0;
		}
	}
}

BUILT_IN_COMMAND(fuckem)
{
	ChannelList *chan;
	BanList *Bans;
	int server;
	char buffer[BIG_BUFFER_SIZE];
	char c;

	if (!(chan = prepare_command(&server, NULL, NEED_OP)))
		return;

	for (Bans = chan->bans; Bans; Bans = Bans->next)
		add_mode(chan, "b", 0, Bans->ban, NULL, get_int_var(NUM_BANMODES_VAR));
	for (c = 'a'; c <= 'z'; c++)
	{
		sprintf(buffer, "*!*@*%c*", c);
		add_mode(chan, "b", 1, buffer, NULL, get_int_var(NUM_BANMODES_VAR));
	}         
	flush_mode_all(chan);
}

/*
 * Lamer Kick!   Kicks All UnOpped People from Current Channel        
 */
BUILT_IN_COMMAND(LameKick)
{
	ChannelList *chan;
	NickList *tmp;
	char *channel = NULL;
	int old_server = from_server;

	if (args && *args && is_channel(args))
		channel = next_arg(args, &args);	
	if ((chan = prepare_command(&from_server, channel, NEED_OP)))
	{
		int count = 0, total = 0;
		char *targets = NULL;
		const char * const reason = args ? args : empty_string;
		const size_t cmd_len = strlen(chan->channel) + strlen("KICK   :<\002BX\002-LK> ") + strlen(reason);

		for (tmp = next_nicklist(chan, NULL); tmp; tmp = next_nicklist(chan, tmp))
		{
			int level = 0;
			if (tmp->userlist)
				level = ((tmp->userlist->flags | 0xff) & PROT_ALL);
			if (!nick_isop(tmp) && !nick_isvoice(tmp) && ((tmp->userlist && !level) || !tmp->userlist))
			{
				m_s3cat(&targets, ",", tmp->nick);
				count++;
				total++;
				if (((get_int_var(NUM_KICKS_VAR)) && 
					(count >= get_int_var(NUM_KICKS_VAR))) || 
					(strlen(targets) + cmd_len) >= (MAX_PROTOCOL_SIZE - (NICKNAME_LEN + 5)))
				{
					my_send_to_server(from_server, "KICK %s %s :<\002BX\002-LK> %s", chan->channel, targets, reason);
					new_free(&targets);
					count = 0;
				}
			}
		}
		if (targets)
			my_send_to_server(from_server, "KICK %s %s :<\002BX\002-LK> %s", chan->channel, targets, reason);
		new_free(&targets);
		say("Sent the Server all the Lamer Kicks, Sit back and Watch %d kicks!", total);
	}
	from_server = old_server;
}

static void shitlist_erase(ShitList **clientlist)
{
	ShitList *Client, *tmp;
	
	for (Client = *clientlist; Client;)
	{
		new_free(&Client->filter);
		new_free(&Client->reason);
		tmp = Client->next;
		new_free((char **)&Client);
		Client = tmp;
	}
	*clientlist = NULL;
}

static char *screw(char *user)
{
	char *p;
	for (p = user; p && *p;)
	{
		switch(*p)
		{
			case '.':
			case ':':
			case '*':
			case '@':
			case '!':
				p+=1;
				break;
			default:
				*p = '?';
				if (*(p+1) && *(p+2))
					p+=2;
				else
					p++;
		}
	}
	return user;
}

char * ban_it(char *nick, char *user, char *host, char *ip)
{
	char *t = user, *t1 = user, *tmp;
	static char banstr[BIG_BUFFER_SIZE/4+1];
	
	while (strlen(t1)>9)
		t1++;
	t1 = clear_server_flags(t1);
	switch (defban) 
	{
		case 7:
			if (ip)
			{
				snprintf(banstr, sizeof banstr, "*!*@%s",
					cluster(ip));
				break;
			}
		case 2: /* Better 	*/
			snprintf(banstr, sizeof banstr, "*!*%s@%s", t1, 
				cluster(host));
			break;
		case 3: /* Host 	*/
			snprintf(banstr, sizeof banstr, "*!*@%s", host);
			break;
		case 4: /* Domain	*/
			tmp = strrchr(host, '.');
			if (tmp) {
				snprintf(banstr, sizeof banstr, "*!*@*%s",
					tmp);
			} else {
				snprintf(banstr, sizeof banstr, "*!*@%s", 
					host);
			}
			break;	
		case 5: /* User		*/
			snprintf(banstr, sizeof banstr, "*!%s@%s", t, 
				cluster(host));
			break;
		case 6: /* Screw 	*/
			snprintf(banstr, sizeof banstr, "*!*%s@%s", t1, host);
			screw(banstr);
			break;
		case 1:	/* Normal 	*/
		default:
			snprintf(banstr, sizeof banstr, "%s!*%s@%s", nick, t1,
				host);
			break;
	}
	return banstr;
}

/* userhost_unban()
 *
 * userhostbase() callback for /UNBAN <nick>.
 * Expected 'args' are channel and server number.  Uses userhost or cached
 * whowas information to unset all matching bans on the channel.
 */
void userhost_unban(UserhostItem *uhi, char *nick, char *args)
{
	ChannelList *chan;
	BanList *bans;
	WhowasList *whowas;
	NickList *n = NULL;
	char *channel, *ip_str = NULL, *host = NULL;
	int count = 0;
	int server = -1;
	
	channel = next_arg(args, &args);
	if (args && *args)
		server = atoi(args);

	/* Should not happen, indicates a bug in the code that setup this callback. */
	if (!channel || server == -1)
		return;

	set_display_target(channel, LOG_CRAP);

	if (!(chan = lookup_channel(channel, server, 0)) || (!chan->have_op && !chan->hop))
	{
		bitchsay("No longer opped on channel %s", channel);
		reset_display_target();
		return;
	}

	if (uhi && uhi->nick && strcmp(uhi->user, "<UNKNOWN>") && !my_stricmp(uhi->nick, nick))
	{
		host = m_sprintf("%s!%s@%s", uhi->nick, uhi->user, uhi->host); 
		n = find_nicklist_in_channellist(uhi->nick, chan, 0);
	}
	else if ((whowas = check_whowas_nick_buffer(nick, channel)))
	{
		n = whowas->nicklist;
		host = m_sprintf("%s!%s", n->nick, n->host);
		bitchsay("Using WhoWas info for unban of %s", nick);
	}
	else
	{
		bitchsay("No matching nick for the unban of %s on %s", nick, channel);
		reset_display_target();
		return;
	}

	if (n && n->ip)
	{
		char *user = m_strdup(n->host);
		char *p = strchr(user, '@');

		if (p)
			*p = 0;

		ip_str = m_sprintf("%s!%s@%s", n->nick, user, n->ip);
		new_free(&user);
	}

	for (bans = chan->bans; bans; bans = bans->next)
	{
		if (!bans->sent_unban && (wild_match(bans->ban, host) || (ip_str && wild_match(bans->ban, ip_str))) )
		{
			add_mode(chan, "b", 0, bans->ban, NULL, get_int_var(NUM_BANMODES_VAR));
			bans->sent_unban++;
			count++;
		}			
	}	

	flush_mode_all(chan);
	if (!count)
		bitchsay("No matching bans for %s on %s", host, channel);
	new_free(&host);
	new_free(&ip_str);
	reset_display_target();
}

void userhost_ban(UserhostItem *stuff, char *nick1, char *args)
{
	ChannelList *c = NULL;
	NickList *n = NULL;
	WhowasList *whowas = NULL;
	char *channel, *nick, *user, *host, *b = "+b", *ob = "-o+b", *str = NULL;
	int fuck, set_ignore, on_chan = 0;
	
	channel = next_arg(args, &args);

	fuck = !my_stricmp("FUCK", args);
	set_ignore = !my_stricmp("BKI", args);
	
	if (!stuff || !stuff->nick || !strcmp(stuff->user, "<UNKNOWN>") || my_stricmp(stuff->nick, nick1))
	{
		if (channel && (whowas = check_whowas_nick_buffer(nick1, channel)))
		{
			nick = whowas->nicklist->nick;
			user = m_strdup(clear_server_flags(whowas->nicklist->host));
			host = strchr(user, '@');
			*host++ = 0;
			bitchsay("Using WhoWas info for ban of %s ", nick1);
			n = whowas->nicklist;
		}
		else 
		{
			bitchsay("No match for the %s of %s on %s", fuck ? "Fuck":"Ban", nick1, channel);
			return;
		}
	} 
	else
	{
		nick = stuff->nick;
		user = m_strdup(clear_server_flags(stuff->user));
		host = stuff->host;
	}

	if (!(my_stricmp(nick, get_server_nickname(from_server))))
	{
		bitchsay("Try to kick yourself again!!");
		new_free(&user);
		return;
	}

	on_chan = is_on_channel(channel, from_server, nick);
	c = lookup_channel(channel, from_server, 0);
	if (c && !n)
		n = find_nicklist_in_channellist(nick, c, 0);
	send_to_server("MODE %s %s %s %s", channel, on_chan ? ob : b, on_chan?nick:empty_string, ban_it(nick, user, host, n ? n->ip : NULL));
	if (fuck)
	{
		malloc_sprintf(&str, "%s!*%s@%s %s 3 Auto-Shit", nick, user, host, channel);
#ifdef WANT_USERLIST
		add_shit(NULL, str, NULL, NULL);
#endif
		new_free(&str);
	} else if (set_ignore)
		ignore_nickname(ban_it("*", user, host, NULL)	, IGNORE_ALL, 0);
	new_free(&user);
}

BUILT_IN_COMMAND(multkick)
{
	ChannelList *chan;
	char *to, *temp, *reason;
	int server;

	to = next_arg(args, &args);
	if (to && !is_channel(to))
	{
		if (args && *args)
			to[strlen(to)] = ' ';
		temp = to;
		to = NULL;
	}
	else
		temp = args;
		
	if (!(chan = prepare_command(&server, to, NEED_OP)))
		return;

	if (!temp || !*temp)
		return;

	reason = strchr(temp, ':');
	if (reason)
		*reason++ = 0;

	while (temp && *temp)
	{
		const char *nick = next_arg(temp, &temp);
		const char *this_reason = reason && *reason ? reason : 
			get_kick_reason(nick, get_server_nickname(server));

		my_send_to_server(server, "KICK %s %s :\002%s\002", chan->channel,
			nick, this_reason);
	}
}

BUILT_IN_COMMAND(massdeop)
{
	ChannelList *chan;
	NickList *nicks;
	int all = 0, count = 0, isvoice = 0, maxmodes = get_int_var(NUM_OPMODES_VAR);
	int old_server = from_server;
	char *to, *spec = NULL, *rest = NULL;
	char buffer[BIG_BUFFER_SIZE + 1];

	if (command && !my_stricmp(command, "mdvoice"))
		isvoice = 1;
	
	to = next_arg(args, &args);
	if (to && !is_channel(to))
	{
		spec = to;
		to = NULL;
	}

	if (!(chan = prepare_command(&from_server, to, NEED_OP)))
	{
		from_server = old_server;
		return;
	}

	if (!spec && !(spec = next_arg(args, &args)))
		spec = "*!*@*";
	if (*spec == '-')
	{
		rest = spec;
		spec = "*!*@*";
	}
	else
		rest = args;
	if (rest && !my_stricmp(rest, "-all"))
		all = 1;

	for (nicks = next_nicklist(chan, NULL); nicks; nicks = next_nicklist(chan, nicks))
	{
		sprintf(buffer, "%s!%s", nicks->nick, nicks->host);
		if ((all || (!isvoice && nick_isop(nicks)) || (isvoice && nick_isvoice(nicks))) &&
		    my_stricmp(nicks->nick, get_server_nickname(from_server)) &&
		    wild_match(spec, buffer))
		{
			add_mode(chan, isvoice? "v":"o", 0, nicks->nick, NULL, maxmodes);
			count++;
		}

	}
	flush_mode_all(chan);
	from_server = old_server;
	if (!count)
		say("No matches for %s of %s on %s", command?command:"massdeop", spec, chan->channel);
}

BUILT_IN_COMMAND(doop)
{
	ChannelList *chan;
	char *to, *temp = NULL;
	int count = 0, maxmodes = get_int_var(NUM_OPMODES_VAR), old_server = from_server;

	/* command is mode char to use - if none given, default to op */
	if (!command)
		command = "o";

	to = next_arg(args, &args);
	if (to && !is_channel(to))
	{
		temp = to;
		to = NULL;
	}

	if (!(chan = prepare_command(&from_server, to, NEED_OP)))
	{
		from_server = old_server;
		return;
	}

	if (!temp)
		temp = next_arg(args, &args);

	while (temp && *temp)
	{
		count++;
		add_mode(chan, command, 1, temp, NULL, maxmodes);
		temp = next_arg(args, &args);
	}
	flush_mode_all(chan);
	from_server = old_server;
}

BUILT_IN_COMMAND(dodeop)
{
	ChannelList *chan;
	char *to, *temp = NULL;
	int count = 0, maxmodes = get_int_var(NUM_OPMODES_VAR), server;

	/* command is mode char to use - if none given, default to deop */
	if (!command)
		command = "o";
	
	to = next_arg(args, &args);
	if (to && !is_channel(to))
	{
		temp = to;
		to = NULL;
	}

	if (!(chan = prepare_command(&server, to, NEED_OP)))
		return;

	if (!temp)
		temp = next_arg(args, &args);

	while (temp && *temp)
	{
		count++;
		add_mode(chan, command, 0, temp, NULL, maxmodes);
		temp = next_arg(args, &args);
	}
	flush_mode_all(chan);
}

BUILT_IN_COMMAND(massop)
{
	ChannelList *chan;
	NickList *nicks;
	char *to, *spec = NULL;
	int count = 0, massvoice = 0, maxmodes = get_int_var(NUM_OPMODES_VAR), server;
	char buffer[BIG_BUFFER_SIZE + 1];
	
	if (command)
		massvoice = 1;

	to = next_arg(args, &args);
	if (to && !is_channel(to) )
	{
		spec = to;
		to = NULL;
	}

	if (!(chan = prepare_command(&server, to, NEED_OP)))
		return;

	if (!spec && !(spec = next_arg(args, &args)))
		spec = "*!*@*";
	if (*spec == '-')
		spec = "*!*@*";

	for (nicks = next_nicklist(chan, NULL); nicks; nicks = next_nicklist(chan, nicks))
	{
		sprintf(buffer, "%s!%s", nicks->nick, nicks->host);
		if ((my_stricmp(nicks->nick, get_server_nickname(from_server)) && wild_match(spec, buffer)))
		{
			if (!(nick_isop(nicks) || (massvoice && nick_isvoice(nicks))))
			{
				add_mode(chan, massvoice?"v":"o", 1, nicks->nick, NULL, maxmodes);
				count++;
			}
		}
	}
	flush_mode_all(chan);
	if (!count)
		say("No matches for %s of %s on %s", command? command : "massop", spec, chan->channel);
}

BUILT_IN_COMMAND(masskick)
{
	ChannelList *chan;
	NickList *nicks;
	ShitList *masskick_list = NULL, *new;
	char *to, *spec = NULL, *rest, *buffer = NULL, *q;
	int all = 0, ops = 0, server;
	
	to = next_arg(args, &args);
	if (to && !is_channel(to))
	{
		spec = to;
		to = NULL;
	}

	if (!(chan = prepare_command(&server, to, NEED_OP)))
		return;

	if (spec && strbegins(spec, "-all"))
	{
		all = 1;
		spec = next_arg(args, &args);
	}
	if (spec && strbegins(spec, "-ops"))
	{
		ops = 1;
		spec = next_arg(args, &args);
	}
	if (!spec)
		return;
	rest = args;
	if (rest && !*rest)
		rest = NULL;

	for (nicks = next_nicklist(chan, NULL); nicks; nicks = next_nicklist(chan, nicks))
	{
		int doit = 0;
		q = clear_server_flags(nicks->host);
		malloc_sprintf(&buffer, "%s!*%s", nicks->nick, q);		
		if (all)
			doit = 1;
		else if (ops && nick_isop(nicks))
			doit = 1;
		else if (!nick_isop(nicks))
			doit = 1;
		else if (get_cset_int_var(chan->csets, KICK_OPS_CSET))
			doit = 1;
		if (doit && !isme(nicks->nick) && (wild_match(spec, buffer) || wild_match(nicks->nick, spec)))
		{
			new = (ShitList *)new_malloc(sizeof(ShitList));
			malloc_sprintf(&new->filter, "%s", nicks->nick);
			add_to_list((List **)&masskick_list, (List *)new);
		}
		new_free(&buffer);
	}

	if (masskick_list)
	{
		int num = 0;
		char *targets = NULL;
		const char * const reason = rest ? rest : "MassKick";
		const size_t cmd_len = strlen(chan->channel) + strlen("KICK   :\002\002") + strlen(reason);

		bitchsay("Performing (%s) Mass Kick on %s", all ? "opz/non-opz" : ops ? "ops":"non-opz", chan->channel);

		for (new = masskick_list; new; new = new->next)
		{
			m_s3cat(&targets, ",", new->filter);
			num++;
			if ((get_int_var(NUM_KICKS_VAR) && (num == get_int_var(NUM_KICKS_VAR))) || 
			    (strlen(targets) + cmd_len) >= (MAX_PROTOCOL_SIZE - (NICKNAME_LEN + 5)))
			{
				num = 0;
				my_send_to_server(server, "KICK %s %s :\002%s\002", chan->channel, targets, reason);
				new_free(&targets);
			}
		}
		if (targets)
			my_send_to_server(server, "KICK %s %s :\002%s\002", chan->channel, targets, reason);
		new_free(&targets);
		shitlist_erase(&masskick_list);
	}
	else
		bitchsay("No matches for mass kick of %s on %s", spec, chan->channel);
}

BUILT_IN_COMMAND(mknu)
{
	ChannelList *chan;
	NickList *nicks;
	char *to = NULL, *rest;
	int count = 0, kickops, server;
	
	if (args && (is_channel(args) || strbegins(args, "* ") || !strcmp(args, "*")))
		to = next_arg(args, &args);
	
	if (!(chan = prepare_command(&server, to, NEED_OP)))
		return;

	rest = args;
	if (rest && !*rest)
		rest = NULL;

	kickops = get_cset_int_var(chan->csets, KICK_OPS_CSET);
	for (nicks = next_nicklist(chan, NULL); nicks; nicks = next_nicklist(chan, nicks))
	{
		if (nicks->userlist || (nick_isop(nicks) && !kickops) || isme(nicks->nick))
			continue;
		count++;
		my_send_to_server(server, "KICK %s %s :(non-users) \002%s\002", 
			chan->channel, nicks->nick, rest ? rest : 
			get_kick_reason(nicks->nick, get_server_nickname(server)));
	}
	if (!count)
		say("No matches for masskick of non-users on %s", chan->channel);
}

BUILT_IN_COMMAND(masskickban)
{
	ChannelList *chan;
	NickList *nicks;
	char *to, *rest, *spec = NULL;
	int all = 0, count = 0, server;
	char buffer[BIG_BUFFER_SIZE + 1], tempbuf[BIG_BUFFER_SIZE + 1];
	
	to = next_arg(args, &args);
	if (to && !is_channel(to))
	{
		spec = to;
		to = NULL;
	}

	if (!(chan = prepare_command(&server, to, NEED_OP)))
		return;

	if (!spec && !(spec = next_arg(args, &args)))
		return;
	if (args && strbegins(args, "-all"))
	{
		all = 1;
		next_arg(args, &args);
	}
	rest = args;
	if (rest && !*rest)
		rest = NULL;

	if (!strchr(spec, '!'))
	{
		strcpy(tempbuf, "*!");
		if (!strchr(spec, '@'))
			strcat(tempbuf, "*@");
		strcat(tempbuf, spec);
	}
	else
		strcpy(tempbuf, spec);

	my_send_to_server(server, "MODE %s +b %s", chan->channel, tempbuf);
	for (nicks = next_nicklist(chan, NULL); nicks; nicks = next_nicklist(chan, nicks))
	{
		*buffer = '\0';
		strmopencat(buffer, IRCD_BUFFER_SIZE, nicks->nick, "!", nicks->host, NULL);
		if ((all || !nick_isop(nicks) || get_cset_int_var(chan->csets, KICK_OPS_CSET)) &&
		   !isme(nicks->nick) && wild_match(tempbuf, buffer))
		{
			count++;
			my_send_to_server(server, "KICK %s %s :(%s) \002%s\002", chan->channel, 
				nicks->nick, spec, rest ? rest : 
				get_kick_reason(nicks->nick, get_server_nickname(server)));
		}
	}
	if (!count)
		say("No matches for masskickban of %s on %s", spec, chan->channel);
}

BUILT_IN_COMMAND(massban)
{
	ChannelList *chan;
	NickList *nicks;
	ShitList *massban_list = NULL, *tmp;
	char *to, *rest, *spec = NULL, *buffer = NULL;
	int all = 0, maxmodes = get_int_var(NUM_BANMODES_VAR), server;

	to = next_arg(args, &args);
	if (to && !is_channel(to))
	{
		spec = to;
		to = NULL;
	}

	if (!(chan = prepare_command(&server, to, NEED_OP)))
		return;

	if (!spec && !(spec = next_arg(args, &args)))
		spec = "*!*@*";
	if (*spec == '-')
	{
		rest = spec;
		spec = "*!*@*";
	}
	else
		rest = args;
	if (rest && !my_stricmp(rest, "-all"))
		all = 1;

	for (nicks = next_nicklist(chan, NULL); nicks; nicks = next_nicklist(chan, nicks))
	{
		malloc_sprintf(&buffer, "%s!%s", nicks->nick, nicks->host);
		if ((all || !nick_isop(nicks) || get_cset_int_var(chan->csets, KICK_OPS_CSET)) &&
		    !isme(nicks->nick) && wild_match(spec, buffer))
		{
			char *temp = LOCAL_COPY(nicks->host), *q = clear_server_flags(temp), *p = strchr(temp, '@');
			ShitList *new;

			*p++ = 0;
			new = (ShitList *)new_malloc(sizeof(ShitList));
			malloc_sprintf(&new->filter, "*!*%s@%s ", q, cluster(p));
			add_to_list((List **)&massban_list, (List *)new);
		}
		new_free(&buffer);
	}
	if (massban_list)
	{
		char modestr[100];
		int i = 0;
		
		bitchsay("Performing Mass Bans on %s", chan->channel);
		for (tmp = massban_list; tmp; tmp = tmp->next)
		{
			malloc_strcat(&buffer, tmp->filter);
			modestr[i] = 'b';
			i++;
			if (i > maxmodes)
			{
				modestr[i] = '\0';
				my_send_to_server(server, "MODE %s +%s %s", chan->channel, modestr, buffer);
				i = 0;
				new_free(&buffer);
			}
		}
		modestr[i] = '\0';
		if (buffer && *buffer)
		{
			my_send_to_server(server, "MODE %s +%s %s", chan->channel, modestr, buffer);
			new_free(&buffer);
		}
		shitlist_erase(&massban_list);
	} else
		say("No matches for massban of %s on %s", spec, chan->channel);
}

BUILT_IN_COMMAND(unban)
{
	ChannelList *chan;
	BanList *bans;
	char *to, *spec = NULL;
	int count = 0, server;
	
	to = new_next_arg(args, &args);
	if (to && !is_channel(to))
	{
		spec = to;
		to = NULL;
	}

	if (!(chan = prepare_command(&server, to, NEED_OP)))
		return;

	set_display_target(chan->channel, LOG_CRAP);
	if (!spec && !(spec = next_arg(args, &args)))
		spec = "*";
	if (spec && *spec == '#')
		count = atoi(spec + 1);
	else if (!strchr(spec, '*'))
	{
		userhostbase(spec, userhost_unban, 1, "%s %d", chan->channel, server);
		reset_display_target();
		return;
	}

	if (count)
	{
		int tmp = 1;
		for (bans = chan->bans; bans; bans = bans->next)
		{
			if (tmp == count)
			{
				if (bans->sent_unban == 0)
				{
					my_send_to_server(server, "MODE %s -b %s", chan->channel, bans->ban);
					bans->sent_unban++;
					tmp = 0;
					break;
				}
			} else
				tmp++;
		}
		if (tmp != 0)
			count = 0;
	} 
	else 
	{
		char *banstring = NULL;
		int num = 0;
		for (bans = chan->bans; bans; bans = bans->next)
		{
			if (wild_match(bans->ban, spec) || wild_match(spec, bans->ban))
			{
				if (bans->sent_unban == 0)
				{
					m_s3cat(&banstring, space, bans->ban);
					bans->sent_unban++;
					count++;
					num++;
				}
			}
			if (count && (count % get_int_var(NUM_BANMODES_VAR) == 0))
			{
				my_send_to_server(server, "MODE %s -%s %s", chan->channel, strfill('b', num), banstring);
				new_free(&banstring);
				num = 0;
			}
		}
		if (banstring && num)
			my_send_to_server(server, "MODE %s -%s %s", chan->channel, strfill('b', num), banstring);
		new_free(&banstring);
	}
	if (!count)
		bitchsay("No ban matching %s found", spec);
	reset_display_target();
}

BUILT_IN_COMMAND(dokick)
{
	ChannelList *chan;
	char *to, *reason, *spec = NULL;
	int server;

	to = next_arg(args, &args);
	if (to && !is_channel(to))
	{
		spec = to;
		to = NULL;
	}

	if (!(chan = prepare_command(&server, (to && !strcmp(to, "*"))? NULL : to, NEED_OP)))
		return;

	if (!spec && !(spec = next_arg(args, &args)))
		return;
	if (args && *args)
		reason = args;
	else
		reason = get_kick_reason(spec, get_server_nickname(server));

	set_display_target(chan->channel, LOG_KICK);
	my_send_to_server(server, "KICK %s %s :%s", chan->channel, spec, reason);
	reset_display_target();
}

BUILT_IN_COMMAND(kickban)
{
	ChannelList *chan;
	NickList *nicks;
	char *to, *tspec, *tnick, *rest, *spec = NULL;
	int count = 0, kick_first = 0, set_ignore = 0, server;
	int time = -1;
	
	to = next_arg(args, &args);
	if (to && !is_channel(to))
	{
		spec = to;
		to = NULL;
	}

	if (!(chan = prepare_command(&server, to, NEED_OP)))
		return;

	set_display_target(chan->channel, LOG_KICK);
	if (command)
	{
		if (strstr(command, "KB"))
			kick_first = 1;
		set_ignore = (command[strlen(command)-1] == 'I');
	}
	if (!spec && !(spec = new_next_arg(args, &args)))
	{
		reset_display_target();
		return;
	}

	if (command && (!my_stricmp(command, "TBK") || !my_stricmp(command, "TKB")))
	{
		char *string_time = next_arg(args, &args);
		if (string_time)
			time = atoi(string_time);
		if (time < 0)
			time = get_cset_int_var(chan->csets, BANTIME_CSET);
		rest = args;
		if (!rest || !*rest)
			rest = m_sprintf("Timed kickban for %s", convert_time(time));
	}
	else
	{
		rest = args;
		if (rest && !*rest)
			rest = NULL;
	}
	tspec = LOCAL_COPY(spec);
	while ((tnick = next_in_comma_list(tspec, &tspec)))
	{
		int exact;
		if (!tnick || !*tnick) break;
		exact = strchr(tnick, '*') ? 0 : 1;
		for (nicks = next_nicklist(chan, NULL); nicks; nicks = next_nicklist(chan, nicks))
		{
			char *p, *user, *host;
			if (isme(nicks->nick))
				continue;
			if (exact && my_stricmp(nicks->nick, tnick))
				continue;
			else if (!exact && !wild_match(tnick, nicks->nick))
				continue;
			p = LOCAL_COPY(nicks->host);
			user = clear_server_flags(p);
			host = strchr(user, '@');
			*host++ = 0;
			if (kick_first)
				my_send_to_server(server, "KICK %s %s :%s\r\nMODE %s +b %s", 
					chan->channel, nicks->nick, rest ? rest : get_kick_reason(nicks->nick, get_server_nickname(server)),
					chan->channel, ban_it(nicks->nick, user, host, nicks->ip));
			else
				my_send_to_server(server, "MODE %s -o+b %s %s\r\nKICK %s %s :%s", 
					chan->channel, nicks->nick, ban_it(nicks->nick, user, host, nicks->ip),
					chan->channel, nicks->nick, rest ? rest : get_kick_reason(nicks->nick, get_server_nickname(server)));
			count++;
			if (time >= 0)
				add_timer(0, empty_string, time * 1000.0, 1, timer_unban, 
					m_sprintf("%d %s %s", from_server, chan->channel, ban_it(nicks->nick, user, host, nicks->ip)),
					NULL, -1, "timed-unban");
			else if (command && !my_stricmp(command, "FUCK"))
			{
				char *temp = NULL;
				malloc_sprintf(&temp, "%s!*%s@%s %s 3 Auto-Shit", nicks->nick, user, host, chan->channel);
#ifdef WANT_USERLIST
				add_shit(NULL, temp, NULL, NULL);
#endif
				new_free(&temp);
			} else if (set_ignore)
				ignore_nickname(ban_it("*", user, host, NULL), IGNORE_ALL, 0);
		}
	}
	if (!count && !isme(spec))
		userhostbase(spec, userhost_ban, 1, "%s %s", chan->channel, command ? (!strcmp(command, "FUCK") ? "FUCK": set_ignore ? "BKI":empty_string):empty_string);
	reset_display_target();
}

BUILT_IN_COMMAND(ban)
{
	ChannelList *chan;
	NickList *nicks;
	char *to, *rest, *spec = NULL;
	int found = 0, server;
	
	to = next_arg(args, &args);
	if (to && !is_channel(to))
	{
		spec = to;
		to = NULL;
	}

	if (!(chan = prepare_command(&server, to, NEED_OP)))
		return;

	if (!spec && !(spec = new_next_arg(args, &args)))
		return;
	rest = args;
	if (rest && !*rest)
		rest = NULL;

	for (nicks = next_nicklist(chan, NULL); nicks; nicks = next_nicklist(chan, nicks))
	{
		if (!my_stricmp(spec, nicks->nick))
		{
			char *t = LOCAL_COPY(nicks->host), *user = clear_server_flags(t), *host = strchr(user, '@');

			*host++ = 0;
			my_send_to_server(server, "MODE %s -o+b %s %s", chan->channel, nicks->nick, ban_it(nicks->nick, user, host, nicks->ip));
			found++;
		}
	}
	if (!found)
	{
		if (strchr(spec, '!') && strchr(spec, '@'))
			my_send_to_server(server, "MODE %s +b %s", chan->channel, spec);
		else
			userhostbase(spec, userhost_ban, 1, "%s", chan->channel);
	}
}

BUILT_IN_COMMAND(banstat)
{
	ChannelList *chan;
	BanList *tmpc;
	char *tmp, *channel = NULL, *check = NULL;
	int count = 1, server;
	
	if ((tmp = next_arg(args, &args)))
	{
		if (is_channel(tmp))
			malloc_strcpy(&channel, tmp);
		else
			malloc_strcpy(&check, tmp);
		if (channel && (tmp = next_arg(args, &args)))
			malloc_strcpy(&check, tmp);
	}

	if ((chan = prepare_command(&server, channel, NO_OP)))
	{
		if (!chan->bans && !chan->exemptbans)
		{
			bitchsay("No bans on %s", chan->channel);
			return;
		}
		if (chan->bans)
		{
			if ((do_hook(BANS_HEADER_LIST, "%s %s %s %s %s", "#", "Channel", "Ban", "SetBy", "Seconds")))
				put_it("%s", convert_output_format(fget_string_var(FORMAT_BANS_HEADER_FSET), NULL));
			for (tmpc = chan->bans; tmpc; tmpc = tmpc->next, count++)
			{
				if (check && (!wild_match(check, tmpc->ban) || !wild_match(tmpc->ban, check)))
					continue;
				if (do_hook(BANS_LIST, "%d %s %s %s %lu", count, chan->channel, tmpc->ban, tmpc->setby?tmpc->setby:get_server_name(from_server), (unsigned long)tmpc->time))
					put_it("%s", convert_output_format(fget_string_var(FORMAT_BANS_FSET), "%d %s %s %s %l", count, chan->channel, tmpc->ban, tmpc->setby?tmpc->setby:get_server_name(from_server), (unsigned long)tmpc->time));
			}
			if (do_hook(BANS_FOOTER_LIST, "%d %s", count, chan->channel))
				put_it("%s", convert_output_format(fget_string_var(FORMAT_BANS_FOOTER_FSET), "%d %s", count, chan->channel));
		}
		if (chan->exemptbans)
		{
			count = 1;
			if ((do_hook(EBANS_HEADER_LIST, "%s %s %s %s %s", "#", "Channel", "ExBan", "SetBy", "Seconds")))
				put_it("%s", convert_output_format(fget_string_var(FORMAT_EBANS_HEADER_FSET), NULL));
			for (tmpc = chan->exemptbans; tmpc; tmpc = tmpc->next, count++)
			{
				if (check && (!wild_match(check, tmpc->ban) || !wild_match(tmpc->ban, check)))
					continue;
				if (do_hook(EBANS_LIST, "%d %s %s %s %lu", count, chan->channel, tmpc->ban, tmpc->setby?tmpc->setby:get_server_name(from_server), (unsigned long)tmpc->time))
					put_it("%s", convert_output_format(fget_string_var(FORMAT_EBANS_FSET), "%d %s %s %s %l", count, chan->channel, tmpc->ban, tmpc->setby?tmpc->setby:get_server_name(from_server), (unsigned long)tmpc->time));
			}
			if (do_hook(EBANS_FOOTER_LIST, "%d %s", count, chan->channel))
				put_it("%s", convert_output_format(fget_string_var(FORMAT_EBANS_FOOTER_FSET), "%d %s", count, chan->channel));
		}
		new_free(&check);
		new_free(&channel);
	} 
	else if (channel)
		send_to_server("MODE %s b", channel);
}

void remove_bans(char *stuff, char *line)
{
	ChannelList *chan;
	BanList *tmpc, *next;
	char *banstring = NULL;
	int count = 1, num = 0, server;
	
	if (stuff && (chan = prepare_command(&server, stuff, NEED_OP)))
	{
		int done = 0;	
		if (!chan->bans)
		{
			bitchsay("No bans on %s", stuff);
			return;
		}

		for (tmpc = chan->bans; tmpc; tmpc = tmpc->next, count++)
			if (!tmpc->sent_unban && (matchmcommand(line, count)))
			{
				malloc_strcat(&banstring, tmpc->ban);
				malloc_strcat(&banstring, space);
				num++;
				tmpc->sent_unban++;
				if (num % get_int_var(NUM_BANMODES_VAR) == 0)
				{
					my_send_to_server(server, "MODE %s -%s %s", stuff, strfill('b', num), banstring);
					new_free(&banstring);
					num = 0;
				}
				done++;
			}
		if (banstring && num)
			my_send_to_server(server, "MODE %s -%s %s", stuff, strfill('b', num), banstring);
		new_free(&banstring);
		num = 0;
		if (!done)
		{
			for (tmpc = chan->exemptbans; tmpc; tmpc = tmpc->next, count++)
				if (!tmpc->sent_unban && (matchmcommand(line, count)))
				{
					malloc_strcat(&banstring, tmpc->ban);
					malloc_strcat(&banstring, space);
					num++;
					tmpc->sent_unban++;
					if (num % get_int_var(NUM_BANMODES_VAR) == 0)
					{
						my_send_to_server(server, "MODE %s -%s %s", stuff, strfill('e', num), banstring);
						new_free(&banstring);
						num = 0;
					}
					done++;
				}
		}
		if (banstring && num)
			my_send_to_server(server, "MODE %s -%s %s", stuff, strfill('b', num), banstring);
		for (tmpc = chan->bans; tmpc; tmpc = next)
		{
			next = tmpc->next;
			if (tmpc->sent_unban)
			{
				if ((tmpc = (BanList *)remove_from_list((List**)&chan->bans, tmpc->ban)))
				{
					new_free(&tmpc->ban);
					new_free(&tmpc->setby);
					new_free((char **)&tmpc);
					tmpc = NULL;
				}
			}
		}
		for (tmpc = chan->exemptbans; tmpc; tmpc = next)
		{
			next = tmpc->next;
			if (tmpc->sent_unban)
			{
				if ((tmpc = (BanList *)remove_from_list((List**)&chan->exemptbans, tmpc->ban)))
				{
					new_free(&tmpc->ban);
					new_free(&tmpc->setby);
					new_free((char **)&tmpc);
					tmpc = NULL;
				}
			}
		}
		new_free(&banstring);
	}	
}

BUILT_IN_COMMAND(tban)
{
	ChannelList *chan;
	BanList *tmpc;
	int count = 1, server;
	
	if ((chan = prepare_command(&server, NULL, NEED_OP)))
	{
		if (!chan->bans)
		{
			bitchsay("No bans on %s", chan->channel);
			return;
		}
		if ((do_hook(BANS_HEADER_LIST, "%s %s %s %s %s", "#", "Channel", "Ban", "SetBy", "Seconds")))
			put_it("%s", convert_output_format(fget_string_var(FORMAT_BANS_HEADER_FSET), NULL));
		for (tmpc = chan->bans; tmpc; tmpc = tmpc->next, count++)
			if (do_hook(BANS_LIST, "%d %s %s %s %lu", count, chan->channel, tmpc->ban, tmpc->setby?tmpc->setby:get_server_name(from_server), (unsigned long)tmpc->time))
				put_it("%s", convert_output_format(fget_string_var(FORMAT_BANS_FSET), "%d %s %s %s %l", count, chan->channel, tmpc->ban, tmpc->setby?tmpc->setby:get_server_name(from_server), (unsigned long)tmpc->time));
		for (tmpc = chan->exemptbans; tmpc; tmpc = tmpc->next, count++)
			if (do_hook(BANS_LIST, "%d %s %s %s %lu", count, chan->channel, tmpc->ban, tmpc->setby?tmpc->setby:get_server_name(from_server), (unsigned long)tmpc->time))
				put_it("%s", convert_output_format(fget_string_var(FORMAT_BANS_FSET), "%d %s %s %s %l", count, chan->channel, tmpc->ban, tmpc->setby?tmpc->setby:get_server_name(from_server), (unsigned long)tmpc->time));
		add_wait_prompt("Which ban to delete (-2, 2-5, ...) ? ", remove_bans, chan->channel, WAIT_PROMPT_LINE, 1);
	} 
}

static const char *bantypes[] = { "*Unknown*", "\002N\002ormal", 
	"\002B\002etter", "\002H\002ost", "\002D\002omain", 
	"\002U\002ser", "\002S\002crew", "\002I\002P" };

static void set_default_bantype(char value, char *helparg)
{
	switch(toupper(value))
	{
		case 'B':
			defban = 2;
			break;
		case 'H':
			defban = 3;
			break;
		case 'D':
			defban = 4;
			break;
		case 'I':
			defban = 7;
			break;
		case 'S':
			defban = 6;
			break;
		case 'U':
			defban = 5;
			break;
		case 'N':
			defban = 1;
			break;
		default :
			return;
	}
	bitchsay("BanType set to %s", 
		bantypes[defban >= 1 && defban <= 7 ? defban : 0]);
}

BUILT_IN_COMMAND(bantype)
{
	if (args && *args)
		set_default_bantype(*args, helparg);
	else
		bitchsay("Current BanType is %s", 
			bantypes[defban >= 1 && defban <= 7 ? defban : 0]);
}
