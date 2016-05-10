/*
 * notice.c: special stuff for parsing NOTICEs
 *
 * Written By Michael Sandrof
 *
 * Copyright(c) 1991
 *
 * See the COPYRIGHT file, or do a HELP IRCII COPYRIGHT 
 */

#include "irc.h"
static char cvsrevision[] = "$Id$";
CVS_REVISION(notice_c)
#include "struct.h"

#include "commands.h"
#include "who.h"
#include "ctcp.h"
#include "window.h"
#include "lastlog.h"
#include "log.h"
#include "flood.h"
#include "vars.h"
#include "ircaux.h"
#include "hook.h"
#include "ignore.h"
#include "server.h"
#include "funny.h"
#include "output.h"
#include "names.h"
#include "parse.h"
#include "notify.h"
#include "misc.h"
#include "screen.h"
#include "status.h"
#include "notice.h"
#include "hash2.h"
#include "cset.h"
#include "input.h"
#define MAIN_SOURCE
#include "modval.h"

extern	char	*FromUserHost;
int	doing_notice = 0;
unsigned long default_swatch = -1;

long	oper_kills = 0,
	nick_collisions = 0,
	serv_fakes = 0,
	serv_unauth = 0,
	serv_squits = 0,
	serv_connects = 0,
	client_connects = 0,
	serv_rehash = 0,
	client_exits = 0,
	serv_klines = 0,
	client_floods = 0,
	client_invalid = 0,
	stats_req = 0,
	client_bot = 0,
	oper_requests = 0;

#ifdef WANT_OPERVIEW

extern void check_orig_nick(char *);
static void handle_oper_vision(const char *from, const char *line)
{
	char arg[4][IRCD_BUFFER_SIZE];
	int up_status = 0;
	const unsigned long flags = get_server_ircop_flags(from_server);
			
	if (!strncmp(line, "*** Notice -- ", 13))
		line += 14;
	else if (!strncmp(line, "*** \002Notice\002 --", 15))
		line += 16;
	else if (!strncmp(line, "*** ", 4))
		line += 4;

/*
[ss]!irc.cs.cmu.edu D-line active for think[think@skateboarders.edu]
*/
	set_display_target(NULL, LOG_SNOTE);	

	/* "Received KILL message for %s. From %s Path: %s (%s)" */	
	if (sscanf(line, "Received KILL message for %500[^.]. From %500s Path: %500s (%500[^)])", arg[0], arg[1], arg[2], arg[3]) == 4)
	{
		enum FSET_TYPES kill_fset;

		check_orig_nick(arg[0]);
		
		if (strchr(arg[1], '.'))
		{
			nick_collisions++;
			if (!(flags & NICK_COLLIDE))
				goto done; 
			kill_fset = FORMAT_SERVER_NOTICE_NICK_COLLISION_FSET;
		}
		else 
		{
			oper_kills++;
			if (!(flags & NICK_KILL))
				goto done;  	

			if (charcount(arg[2], '!') <= 2)
				kill_fset = FORMAT_SERVER_NOTICE_KILL_LOCAL_FSET;
			else
				kill_fset = FORMAT_SERVER_NOTICE_KILL_FSET;
		}
		serversay(from, "%s", convert_output_format(fget_string_var(kill_fset), "%s %s %s %s", update_clock(GET_TIME), arg[1], arg[0], arg[3]));
		up_status = 1;
	}
    /* hybrid: "Nick collision on %s(%s <- %s)(both killed)"
     * bahamut: "Nick collision on %s"
     * ircnet: "Nick collision on %s (%s@%s)%s <- (%s@%s)%s"
     * unreal: "Nick collision on %s (%s %ld <- %s %ld)"
     */
    else if (arg[1][0] = 0, sscanf(line, "Nick collision on %500[^( ] %500[^\n]", arg[0], arg[1]) > 0)
	{
		nick_collisions++;

		if (!(flags & NICK_COLLIDE))
			goto done;  	

		serversay(from, "%s", convert_output_format(fget_string_var(FORMAT_SERVER_NOTICE_NICK_COLLISION_FSET), "%s %s %s", update_clock(GET_TIME), arg[0], arg[1]));
		up_status = 1;
	}
	else if (sscanf(line, "IP# Mismatch: %500[^\n]", arg[0]) == 1)
	{
		if (!(flags & IP_MISMATCH))
			goto done;

		serversay(from, "%s", convert_output_format("IP Mismatch %C$0-", "%s %s", arg[0]));
	}
	else if (sscanf(line, "Hacked ops on opless channel: %500s", arg[0]) == 1)
	{
		if (!(flags & HACK_OPS))
			goto done;

		serversay(from, "%s", convert_output_format("Hacked ops on $0", "%s", arg[0]));
	}
	else if (sscanf(line, "connect failure: %500[^\n]", arg[0]) == 1)
	{
		client_connects++;
		client_exits++;
		if (!(flags & SERVER_CRAP))
			goto done;

		serversay(from, "%s", convert_output_format("Connect failure %K[%n$0-%K]", "%s", arg[0]));
	} 
  	else if (sscanf(line, "Fake: %500s MODE %500[^\n]", arg[0], arg[1]) == 2)
  	{
		serv_fakes++;
		if (!(flags & FAKE_MODE))
			goto done;

		serversay(from, "%s", convert_output_format(fget_string_var(FORMAT_SERVER_NOTICE_FAKE_FSET), "%s %s MODE %s", update_clock(GET_TIME), arg[0], arg[1]));
  	}
	/* "Unauthorized connection from %s." */
  	else if (sscanf(line, "Unauthorized connection from %500s", arg[0]) == 1)
  	{
		serv_unauth++;
		if (!(flags & UNAUTHS))
			goto done;
		
		chop(arg[0], 1);
		serversay(from, "%s", convert_output_format(fget_string_var(FORMAT_SERVER_NOTICE_UNAUTH_FSET), "%s %s", update_clock(GET_TIME), arg[0]));
	}
	/* "Too many connections from %s." */
  	else if (sscanf(line, "Too many connections from %500s", arg[0]) == 1)
  	{
		serv_unauth++;
		if (!(flags & TOO_MANY))
			goto done;

		chop(arg[0], 1);
		serversay(from, "%s", convert_output_format(fget_string_var(FORMAT_SERVER_NOTICE_UNAUTH_FSET), "%s %s", update_clock(GET_TIME), arg[0]));
	}
	/* hybrid-7/ratbox/ircnet/unreal: "%s is rehashing server config file"
	 * hybrid-8: "%s is rehashing configuration file(s)"
	 * bahamut: "%s is rehashing Server config file while whistling innocently"
	 * ircu: "%s is rehashing Server config file"
	 */
	else if (sscanf(line, "%500s is rehashing Server config %500s", arg[0], arg[1]) == 2 ||
		sscanf(line, "%500s is rehashing server config %500s", arg[0], arg[1]) == 2 ||
		sscanf(line, "%500s is rehashing configuration %500s", arg[0], arg[1]) == 2)
	{
		serv_rehash++;
		if (!(flags & REHASH))
			goto done;

		serversay(from, "%s", convert_output_format(fget_string_var(FORMAT_SERVER_NOTICE_REHASH_FSET), "%s %s", update_clock(GET_TIME), arg[0]));
	}
	/* hybrid/ratbox: "%s added K-Line for [%s@%s] [%s]"
	 * bahamut: "%s added "LOCAL_BAN_NAME" for [%s@%s] [%s]"
	 */
	else if (sscanf(line, "%500s added %500s for [%500[^]]] [%500[^\n]", arg[0], arg[1], arg[2], arg[3]) == 4)
	{
		char oper[IRCD_BUFFER_SIZE];
		char serv[IRCD_BUFFER_SIZE];

		serv_klines++;

		if (!(flags & KLINE))
			goto done;

		chop(arg[3], 1);
		/* hybrid/ratbox: get_oper_name() is "%s!%s@%s{%s}" */
		if (sscanf(arg[0], "%100[^!]!%*[^@]@%*[^{]{%100s", oper, serv) == 2 && strchr(serv, '.'))
		{
			chop(serv, 1);
			serversay(from, "%s", convert_output_format(fget_string_var(FORMAT_SERVER_NOTICE_GLINE_FSET), "%s %s %s %s %s %s", update_clock(GET_TIME), oper, arg[2], serv, arg[1], arg[3]));
		}
		else	
			serversay(from, "%s", convert_output_format(fget_string_var(FORMAT_SERVER_NOTICE_KLINE_FSET), "%s %s %s %s %s", update_clock(GET_TIME), arg[0], arg[2], arg[1], arg[3]));
	}	
	/* "User %s (%s@%s) trying to join %s is a possible spambot"
	 * "User %s (%s@%s) is a possible spambot"
	 */
	else if (sscanf(line, "User %500s (%500s is a possible spam%500s", arg[0], arg[1], arg[2]) == 3 ||
		sscanf(line, "User %500s (%500s trying to join %*s is a possible spam%500s", arg[0], arg[1], arg[2]) == 3)
	{
		client_bot++;

		if (!(flags & POSSIBLE_BOT))
			goto done;

		chop(arg[1], 1);
		serversay(from, "%s", convert_output_format(fget_string_var(FORMAT_SERVER_NOTICE_BOT_FSET), "%s %s %s", update_clock(GET_TIME), arg[0], arg[1]));
	}
	/* ircnet/ircu/bahamut: "%s (%s@%s) is now operator (%c)"
	 * hybrid-7/ratbox: "%s (%s@%s) is now an operator"
	 * hybrid-8: "%s!%s@%s{%s} is now an operator"
	 */
	else if (sscanf(line, "%500s (%500s is now oper%500s", arg[0], arg[1], arg[2]) == 3 ||
		sscanf(line, "%500s (%500s is now an oper%500s", arg[0], arg[1], arg[2]) == 3 ||
		sscanf(line, "%500[^!]!%500s is now an oper%500s", arg[0], arg[1], arg[2]) == 3)
	{
		size_t len;

		oper_requests++;

		if (!(flags & OPER_MODE))
			goto done;

		len = strlen(arg[1]);
		if (arg[1][len - 1]	== ')')
			arg[1][len - 1] = 0;

		serversay(from, "%s", convert_output_format(fget_string_var(FORMAT_OPER_FSET), "%s %s %s", update_clock(GET_TIME), arg[0], arg[1]));
	} 
	/* "Received SQUIT %s from %s (%s)" */
	else if (sscanf(line, "Received SQUIT %500s from %500s (%500[^\n]", arg[0], arg[1], arg[2]) == 3)
	{
		serv_squits++;

		if (!(flags & SQUIT))
			goto done;

		chop(arg[2], 1);
		serversay(from, "%s", convert_output_format("SQUIT of $1 from $2 %K[%R$3-%K]", "%s %s %s %s", update_clock(GET_TIME), arg[0], arg[1], arg[2]));
	}
	/* "Received SERVER %s from %s (%d %s)" */
	else if (sscanf(line, "Received SERVER %500s from %500s (%500[^\n]", arg[0], arg[1], arg[2]) == 3)
	{
		serv_connects++;

		if (!(flags & SERVER_CONNECT))
			goto done;

		chop(arg[2], 1);
		serversay(from, "%s", convert_output_format("Received SERVER %c$1%n from %c$2%n %K[%W$3-%K]", "%s %s %s %s", update_clock(GET_TIME), arg[0], arg[1], arg[2]));
	} 
	/* "Sending SQUIT %s (%s)" */
	else if (sscanf(line, "Sending SQUIT %500s (%500[^\n]", arg[0], arg[1]) == 2)
	{
		serv_squits++;

		if (!(flags & SQUIT))
			goto done;

		chop(arg[1], 1);
		serversay(from, "%s", convert_output_format("Sending SQUIT %c$1%n %K[%R$2-%K]", "%s %s %s", update_clock(GET_TIME), arg[0], arg[1]));
	}
	/* "Remote CONNECT %s %d from %s" */
	else if (sscanf(line, "Remote CONNECT %500s %500s from %500s", arg[0], arg[1], arg[2]) == 3)
	{
		serv_connects++;

		if (!(flags & SERVER_CONNECT))
			goto done;

		serversay(from, "%s", convert_output_format("Remote Connect of $1:$2 from $3", "%s %s %s %s", update_clock(GET_TIME), arg[0], arg[1], arg[2]));
	}
	/* hybrid-7, ratbox: "Client connecting: %s (%s@%s) [%s] {%s} [%s]"
     * hybrid-8: "Client connecting: %s (%s@%s) [%s] {%s} [%s] <%s>"
     * ircu: "Client connecting: %s (%s@%s) [%s] {%s} [%s] <%s%s>"
	 * bahamut: "Client connecting: %s (%s@%s) [%s] {%s}"
	 * unreal: "Client connecting on port %d: %s (%s@%s) [%s] %s%s%s"
	 */
	else if (sscanf(line, "Client connecting: %500s (%500[^)]) %500[^\n]", arg[0], arg[1], arg[2]) == 3 ||
		sscanf(line, "Client connecting on port %*[^:]: %500s (%500[^)]) %500[^\n]", arg[0], arg[1], arg[2]) == 3)
	{
		client_connects++;

		if (!(flags & CLIENT_CONNECT))
			goto done;

		serversay(from, "%s", convert_output_format(fget_string_var(FORMAT_SERVER_NOTICE_CLIENT_CONNECT_FSET),
			"%s %s %s %s", update_clock(GET_TIME), arg[0], arg[1], arg[2]));
	}
	/* hybrid, ratbox, bahamut: "Client exiting: %s (%s@%s) [%s] [%s]"
	 * ircu: "Client exiting: %s (%s@%s) [%s] [%s] <%s%s>"
	 * unreal: "Client exiting: %s (%s@%s) [%s]"
	 */
	else if (sscanf(line, "Client exiting: %500s (%500[^)]) %500[^\n]", arg[0], arg[1], arg[2]) == 3)
	{
		client_exits++;

		if (!(flags & CLIENT_CONNECT))
			goto done;

		serversay(from, "%s", convert_output_format(fget_string_var(FORMAT_SERVER_NOTICE_CLIENT_EXIT_FSET),
			"%s %s %s %s", update_clock(GET_TIME), arg[0], arg[1], arg[2]));
	}
	/* bahamut: "Flood -- %s!%s@%s (%d) Exceeds %d RecvQ"
	 * unreal: "Flood -- %s!%s@%s (%d) exceeds %d recvQ"
	 */
	else if (sscanf(line, "Flood -- %500s %500[^\n]", arg[0], arg[1]) == 2)
	{
		client_floods++;

		if (!(flags & TERM_FLOOD))
			goto done;  	

		serversay(from, "%s", convert_output_format(fget_string_var(FORMAT_SERVER_NOTICE_CLIENT_TERM_FSET), "%s %s %s", update_clock(GET_TIME), arg[0], arg[1]));
	}
	/* !ircnet: "Invalid username: %s (%s@%s)" */
	else if (sscanf(line, "Invalid username: %500s (%500[^)])", arg[0], arg[1]) == 2)
	{
		client_invalid++;

		if (!(flags & INVALID_USER))
			goto done;  	

		serversay(from, "%s", convert_output_format(fget_string_var(FORMAT_SERVER_NOTICE_CLIENT_INVALID_FSET), "%s %s %s", update_clock(GET_TIME), arg[0], arg[1]));
	}
	/* ircnet: "Invalid username:  %s@%s." */
	else if (sscanf(line, "Invalid username: %500s", arg[0]) == 1)
	{
		client_invalid++;

		if (!(flags & INVALID_USER))
			goto done;

		chop(arg[0], 1);
		serversay(from, "%s", convert_output_format(fget_string_var(FORMAT_SERVER_NOTICE_CLIENT_INVALID_FSET), "%s - %s", update_clock(GET_TIME), arg[0]));
	}
	/* "STATS %c requested by %s (%s@%s) [%s]" */
	else if (sscanf(line, "STATS %500s requested by %500s (%500[^)]) [%500[^]]]", arg[0], arg[1], arg[2], arg[3]) == 4)
	{
		stats_req++;

		if (!(flags & STATS_REQUEST))
			goto done;  	

		serversay(from, "%s", convert_output_format(fget_string_var(FORMAT_SERVER_NOTICE_STATS_FSET), "%s %s %s %s %s", update_clock(GET_TIME), arg[0], arg[1], arg[2], arg[3]));
	}
	else if (sscanf(line, "Nick flooding detected by: %500[^\n]", arg[0]) == 1)
	{
		if (!(flags & NICK_FLOODING))
			goto done;  	

		serversay(from, "%s", convert_output_format("Nick Flooding %K[%B$1-%K]", "%s %s", update_clock(GET_TIME), arg[0]));
	}
	/* ircu: "K-line active for %s%s"
	 * ircnet: "Kill line active for %s"
	 */
	else if (sscanf(line, "Kill line active for %500[^\n]", arg[0]) == 1 ||
		sscanf(line, "K-line active for %500[^\n]", arg[0]) == 1)
	{
		if (!(flags & KILL_ACTIVE))
			goto done;

		serversay(from, "%s", convert_output_format("Kill line for $1- active", "%s %s", update_clock(GET_TIME), arg[0]));
	}
	else 
	{
		if (!(flags & SERVER_CRAP))
			goto done;  	
		serversay(from, "%s", convert_output_format(fget_string_var(FORMAT_SERVER_NOTICE_FSET), "%s %s %s", update_clock(GET_TIME), from, stripansicodes(line)));
	} 
done:
	reset_display_target();
	if (up_status)
		update_all_status(current_window, NULL, 0);
}
#endif

static void parse_server_notice(const char *from, char *line)
{
	int	flag = 0;
	const char *f = from;
				
	if (!f || !*f)
		f = get_server_itsname(from_server);
			
	if (*line != '*'  && *line != '#' && strncmp(line, "MOTD ", 4))
		flag = 1;
	else
		flag = 0;

	if (do_hook(SERVER_NOTICE_LIST, flag?"%s *** %s":"%s %s", f, line))
	{
#ifdef WANT_OPERVIEW
		handle_oper_vision(f, line);
#else	
		if (!flag)
			next_arg(line, &line);

		set_display_target(NULL, LOG_SNOTE);
		serversay(f, "%s", convert_output_format(
			fget_string_var(FORMAT_SERVER_NOTICE_FSET), "%s %s %s",
			update_clock(GET_TIME), f, stripansicodes(line)));
		reset_display_target();
#endif
	}
}

static int check_ignore_notice(char *from, char *to, unsigned long type, char *line, char **high)
{
	int flag = check_ignore(from, FromUserHost, to, type, line);

	switch (flag)
	{
		case IGNORED:
			doing_notice = 0;
			break;
		case HIGHLIGHTED:
			*high = highlight_char;
			break;
		default:
			*high = empty_string;
	}
	return flag;
}

/* Check if a notice is a BitchX /WALL, and if so handle it as such. */
static int check_chanwall_notice(const char *from, const char *line, int type)
{
	ChannelList *chan = NULL;
	char *line_copy;
	char *newline;
	char *channel = NULL, *p;

	if (!wild_match("[%Wall%/%] *", line))
		return 0;

	line_copy = m_strdup(line);
	p = next_arg(line_copy, &newline);

	if ((p = strchr(p, '/')))
	{
		p++;
		if (*p == BOLD_TOG)
			p++;

		channel = p;

		if ((p = strrchr(channel, ']'))) 
		{
			*p = 0;
			if (p > channel && *(p-1) == BOLD_TOG)
				*(p-1) = 0;
		}
	}

	if (channel && *channel)
		chan = lookup_channel(channel, from_server, CHAN_NOUNLINK);

	if (!chan)
	{
		new_free(&line_copy);
		return 0;
	}

	set_display_target(channel, LOG_WALL);
	
	if (do_hook(type, "%s %s", from, line))
	{
		char *s = convert_output_format(fget_string_var(FORMAT_BWALL_FSET), "%s %s %s %s %s", update_clock(GET_TIME), channel, from, FromUserHost, newline);
		add_to_log(chan->msglog_fp, now, s, logfile_line_mangler);
		put_it("%s", s);
	}
	add_last_type(&last_wall[0], 1, from, FromUserHost, NULL, line);
	logmsg(LOG_WALL, from, 0, "%s", line);
/*	addtabkey(from, "wall", 0);*/
	new_free(&line_copy);
	return 1;
}

void parse_notice(char *from, char **Args)
{
	int	list_type;
	unsigned long ignore_type = IGNORE_NOTICES;
	unsigned long log_type = LOG_NOTICE;
	int flood_type = NOTICE_FLOOD;
	char	*to,
		*high = empty_string,
		*target,
		*line;
	ChannelList *tmpc = NULL;
	
	PasteArgs(Args, 1);
	to = Args[0];
	line = Args[1];
	if (!to || !line)
		return;
	if (!*to)
	{
		put_it("*** obsolete notice received. [%s]", line+1);
		return;
	}

	/* An unprefixed NOTICE or one received before registration is assumed to
	 * be from the local server. */
	if (!from || !*from || !is_server_connected(from_server) || 
	    !strcmp(get_server_itsname(from_server), from))
	{
		parse_server_notice(from, line);
		return;
	}
	
	if (is_channel(to))
	{
		target = to;
		list_type = PUBLIC_NOTICE_LIST;
		if ((tmpc = lookup_channel(to, from_server, CHAN_NOUNLINK)))
		{
			NickList *nick = find_nicklist_in_channellist(from, tmpc, 0);
			update_stats(NOTICELIST, nick, tmpc, 0);		
		}
	}
	else
	{
		target = from;
		if (my_stricmp(to, get_server_nickname(from_server)))
		{
			/* A NOTICE to a global destination like $$*.org */
			log_type = LOG_WALL;
			ignore_type = IGNORE_WALLS;
			list_type = NOTICE_GROUP_LIST;
			flood_type = WALL_FLOOD;
		}
		else
		{
			list_type = NOTICE_LIST;
		}
	}

	set_display_target(target, log_type);
	doing_notice = 1;

	if ((check_ignore_notice(from, to, ignore_type, line, &high) == IGNORED))
		goto notice_cleanup;

	if (!check_flooding(from, flood_type, line, NULL))
		goto notice_cleanup;

	if (!strchr(from, '.') && list_type != NOTICE_GROUP_LIST)
	{
		notify_mark(from, FromUserHost, 1, 0);
		line = do_notice_ctcp(from, to, line);
		if (!*line)
			goto notice_cleanup;
	}
		
	if (sed && !do_hook(ENCRYPTED_NOTICE_LIST, "%s %s %s", from, to, line))
	{
#if 0
		put_it("%s", convert_output_format(fget_string_var(FORMAT_ENCRYPTED_NOTICE_FSET), "%s %s %s %s", update_clock(GET_TIME), from, FromUserHost, line));
#endif
		sed = 0;
		goto notice_cleanup;
	}

	if (!check_chanwall_notice(from, line, list_type))
	{
		char *s;
		switch (list_type)
		{
			case PUBLIC_NOTICE_LIST:
			s = convert_output_format(fget_string_var(check_auto_reply(line)?FORMAT_PUBLIC_NOTICE_AR_FSET:FORMAT_PUBLIC_NOTICE_FSET), "%s %s %s %s %s", update_clock(GET_TIME), from, FromUserHost, to, line);
			break;

			case NOTICE_GROUP_LIST:
			s = convert_output_format(fget_string_var(FORMAT_NOTICE_GROUP_FSET), "%s %s %s %s", update_clock(GET_TIME), from, to, line);
			break;

			default:
			s = convert_output_format(fget_string_var(FORMAT_NOTICE_FSET), "%s %s %s %s", update_clock(GET_TIME), from, FromUserHost, line);
		}
		switch (list_type)
		{
			case PUBLIC_NOTICE_LIST:
			case NOTICE_GROUP_LIST:
			if (do_hook(list_type, "%s %s %s", from, to, line))
				put_it("%s", s);
			break;

			default:
			if (do_hook(list_type, "%s %s", from, line))
				put_it("%s", s);
		}

		if (tmpc)
			add_to_log(tmpc->msglog_fp, now, s, logfile_line_mangler);
		logmsg(log_type, from, 0, "%s", line);
		add_last_type(&last_notice[0], MAX_LAST_MSG, from, FromUserHost, to, line);
	}

 notice_cleanup:
	if (beep_on_level & log_type)
		beep_em(1);
	reset_display_target();
	doing_notice = 0;
}


int loading_savefile = 0;

void load_scripts(void)
{
	extern char *new_script;
	static int done = 0;
#if !defined(WINNT) && !defined(__EMX__)
	char buffer[BIG_BUFFER_SIZE+1];
	int old_display = window_display;
#endif
	if (!done++)
	{
		never_connected = 0;
#if !defined(WINNT) && !defined(__EMX__)
		window_display = 0;
		sprintf(buffer, "%s/bxglobal", SCRIPT_PATH);
		loading_global = 1;
		load("LOAD", buffer, empty_string, NULL);
		loading_global = 0;
		window_display = old_display;
#endif


		if (!quick_startup)
		{
			loading_savefile++;
			reload_save(NULL, NULL, empty_string, NULL);
			loading_savefile--;
			/* read the newscript/.bitchxrc/.ircrc file */
			if (new_script && !access(new_script, R_OK))
				load("LOAD", new_script, empty_string, NULL);
			else if (!access(bircrc_file, R_OK))
				load("LOAD", bircrc_file, empty_string, NULL);
			else if (!access(ircrc_file, R_OK))
				load("LOAD", ircrc_file, empty_string, NULL);
		}
	}
	if (get_server_away(from_server))
		set_server_away(from_server, get_server_away(from_server), 1);
}

#define IGNORE_DONT 1

void setup_ov_mode(int on, int hide, int log)
{
#ifdef WANT_OPERVIEW 

char *default_oper = "wsckf";
Window *win = NULL;

	if (on)
	{
		if ((win = get_window_by_name("oper_view")))
		{
			if (win->log)
			do_log(0, NULL, &win->log_fp);
			delete_window(win);
			update_all_windows();
			set_input_prompt(current_window, get_string_var(INPUT_PROMPT_VAR), 0);
			cursor_to_input();
		}
		send_to_server("MODE %s -%s%s", get_server_nickname(from_server), get_string_var(OPER_MODES_VAR)?get_string_var(OPER_MODES_VAR):default_oper, send_umode);
	} 
	else 
	{
		Window *tmp = NULL;
		win = current_window;
		if ((tmp = new_window(current_window->screen)))
		{
			malloc_strcpy(&tmp->name, "oper_view");
			tmp->double_status = 0;
			if (hide)
				hide_window(tmp);
			else
				resize_window(1, tmp, -5);
			tmp->window_level = LOG_WALLOP|LOG_OPNOTE|LOG_SNOTE;
			tmp->absolute_size = 1;
			tmp->skip = 1;
			set_wset_string_var(tmp->wset, STATUS_FORMAT1_WSET, fget_string_var(FORMAT_OV_FSET));
			build_status(tmp, NULL, 0);
			update_all_windows();
			set_input_prompt(current_window, get_string_var(INPUT_PROMPT_VAR), 0);
			cursor_to_input();
			send_to_server("MODE %s +%s", get_server_nickname(from_server), get_string_var(OPER_MODES_VAR)?get_string_var(OPER_MODES_VAR):default_oper);
			set_screens_current_window(win->screen, win);
			tmp->mangler = operlog_line_mangler;
			if (log != -1)
			{
				tmp->log = log;
				if (tmp->log)
					do_log(log, "~/.BitchX/operview.log", &tmp->log_fp);
			}
		}
	}
#endif
}


#ifdef WANT_OPERVIEW 
char *opflags[] = {"COLLIDE", "KILLS", "MISMATCH", "HACK", "IDENTD", "FAKES",
		"UNAUTHS", "CLIENTS", "TRAFFIC", "REHASH", "KLINE", "BOTS", 
		"OPER", "SQUIT", "SERVER", "CONNECT", "FLOOD", "USER", "STATS",
		"NICK",	"ACTIVEK", "CRAP", NULL};

char	all[] = "ALL",
	none[] = "NONE";

unsigned long ircop_str_to_flags(unsigned org_flags, char *str)
{
unsigned long flag = org_flags;
int neg = 0;
char *ptr;
int i, j;
	while ((ptr = next_in_comma_list(str, &str)))
	{
		if (!ptr || !*ptr)
			return flag;
		switch(*ptr)
		{
			case '-':
				neg = IGNORE_DONT;
				ptr++;
				break;
			default:
				neg = 0;
		}
		upper(ptr);
		if (!strcmp(ptr, all))
		{
			switch(neg)
			{
				case IGNORE_DONT:
					flag &= (~-1);
					break;
				default:
					flag = -1;
					break;
				}
		}
		if (!strcmp(ptr, none))
			return 0;
		for (i = 0, j = 1; opflags[i]; i++, j <<= 1 )
		{
			if (!strcmp(ptr, opflags[i]))
			{
				switch(neg)
				{
					case IGNORE_DONT:
						flag &= (~j);
						break;
					default:
						flag |= j;
						break;
				}
				break;
			}
		}
	}
	return flag;
}

char *ircop_flags_to_str(long flag, char *buffer, size_t len)
{
	int i;

	buffer[0] = 0;
	for (i = 0; opflags[i]; i++)
	{
		if (flag & (1L << i))
		{
			strlcat(buffer, opflags[i], len);
			strlcat(buffer, ",", len);
		}
	}
	return chop(buffer, 1);
}

void print_ircop_flags(int server)
{
	char buffer[200];
	long flag = get_server_ircop_flags(server);

	ircop_flags_to_str(flag, buffer, sizeof buffer);
	put_it("%s", convert_output_format("$G %bOper%BView%n: $0-", "%s", *buffer ? flag == -1 ? "ALL" : buffer : "NONE"));
}

void convert_swatch(Window *win, char *str, int unused)
{
	unsigned long flag;
	char buffer[200];

	if (from_server != -1)
	{
		flag = ircop_str_to_flags(get_server_ircop_flags(from_server), str);
		set_server_ircop_flags(from_server, flag);
	}
	else
		flag = ircop_str_to_flags(default_swatch, str);
	default_swatch = flag;
	ircop_flags_to_str(flag, buffer, sizeof buffer);
	set_string_var(SWATCH_VAR, buffer);
}

void set_operview_flags(int server, unsigned long flags, int neg)
{
	unsigned long old_flags = get_server_ircop_flags(server);

	switch (neg)
	{
		case IGNORE_DONT:
			set_server_ircop_flags(server, old_flags & ~flags);
			break;
		default:
			set_server_ircop_flags(server, old_flags | flags);
			break;
	}
}

BUILT_IN_COMMAND(s_watch)
{
	int gotargs = 0;
	unsigned long old_flags, flag;

	if (from_server == -1)
	{
		put_it("%s", convert_output_format("$G Try connecting to a server first", NULL, NULL));
		return;
	}

	if (args && *args)
		gotargs = 1;

	old_flags = get_server_ircop_flags(from_server);	
	flag = ircop_str_to_flags(old_flags, args);
	if (flag != old_flags)
		set_server_ircop_flags(from_server, flag);
	else if (gotargs && old_flags != -1)
	{
		int i;
		char buffer[BIG_BUFFER_SIZE];

		strcpy(buffer, all);
		for (i = 0; opflags[i]; i++)
		{
			strlcat(buffer, space, sizeof buffer);
			strlcat(buffer, opflags[i], sizeof buffer);
		}
		strlcat(buffer, space, BIG_BUFFER_SIZE);
		strlcat(buffer, none, BIG_BUFFER_SIZE);
		bitchsay("You must specify from the following:");
		put_it("\t%s", buffer);
		return;
	}
	print_ircop_flags(from_server);
}

extern int old_ov_mode;
BUILT_IN_COMMAND(ov_window)
{
char *arg;
int ov = get_int_var(OV_VAR);
static int hide = DEFAULT_OPERVIEW_HIDE;
int count = 0;
int old_hide = hide;  
char *number = NULL;
int log = 0;
	while ((arg = next_arg(args, &args)))
	{
		if (!my_stricmp(arg, on))
			ov = 1;
		else if (!my_stricmp(arg, off))
			ov = 0;
		else if (!my_stricmp(arg, "+HIDE"))
			hide = 1;
		else if (!my_stricmp(arg, "-HIDE"))
			hide = 0;
		else if (!my_stricmp(arg, "HIDE"))
			hide = 1;
		else if (!my_stricmp(arg, "GROW"))
			number = next_arg(args, &args);
		else if (!my_stricmp(arg, "LOG"))
			log ^= 1;
		count++;
	}
	if (count == 0)
		put_it("%s", convert_output_format("$G %BOper%bView%n is %K[%W$0%K]", "%s", on_off(get_int_var(OV_VAR))));
	else
	{
		if ((!ov && get_int_var(OV_VAR)) || (ov && !get_int_var(OV_VAR)))
		{
			setup_ov_mode(ov ?  0 : 1, hide, log);
			old_ov_mode = ov;
			set_int_var(OV_VAR, ov);
			put_it("%s", convert_output_format("$G %BOper%bView%n is now toggled %K[%W$0%K]", "%s", on_off(get_int_var(OV_VAR))));
			return;
		} 
		if (get_int_var(OV_VAR) && old_hide != hide)
		{
			Window *tmp = get_window_by_name("oper_view");
			Window *old_window = current_window;
			if (tmp)
			{
				if (!old_hide && hide)
					hide_window(tmp);
				else
				{
					show_window(tmp);
					resize_window(1, tmp, -5);
				}
				update_all_windows();
				cursor_to_input();
				if (old_window)
					set_screens_current_window(old_window->screen, old_window);
			}
			return;
		}
		if (hide == 0 && number && is_number(number))
		{
			Window *tmp = get_window_by_name("oper_view");
			if (tmp)
				resize_window(1, tmp, my_atol(number));
			update_all_windows();
			return;
		}
 		put_it("%s", convert_output_format("$G %BOper%bView%n is already %K[%W$0%K]", "%s", on_off(get_int_var(OV_VAR))));
		hide = old_hide;
		{
			char buffer[BIG_BUFFER_SIZE] = "~/.BitchX/operview.log";
			Window *tmp = get_window_by_name("oper_view");
			if (tmp)
			{
				tmp->log = log;
				do_log(tmp->log, buffer, &tmp->log_fp); 
				if (!tmp->log_fp)
					tmp->log = 0;
				tmp->mangler = operlog_line_mangler;
			}
		}
		
	}
}
#endif
