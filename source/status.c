/*
 * status.c: handles the status line updating, etc for IRCII 
 *
 * Written By Michael Sandrof
 *
 * Copyright(c) 1990 
 *
 * See the COPYRIGHT file, or do a HELP IRCII COPYRIGHT 
 */


#include "irc.h"
static char cvsrevision[] = "$Id$";
CVS_REVISION(status_c)
#include "struct.h"

#include "ircterm.h"
#include "status.h"
#include "server.h"
#include "vars.h"
#include "hook.h"
#include "input.h"
#include "commands.h"
#include "window.h"
#include "screen.h"
#include "mail.h"
#include "output.h"
#include "names.h"
#include "ircaux.h"
#include "misc.h"
#include "notify.h"
#include "hash2.h"
#include "cset.h"
#ifdef TRANSLATE
#include "translat.h"
#endif
#define MAIN_SOURCE
#include "modval.h"
#ifdef WANT_HEBREW
#include "hebrew.h"
#endif

#define MY_BUFFER 120

extern char *DCC_get_current_transfer (void);
extern	long	oper_kills;
extern	long	nick_collisions;

static	char	*convert_format (Window *, char *, int);
static	char	*status_nickname (Window *);
static	char	*status_query_nick (Window *);
static	char	*status_right_justify (Window *);
static	char	*status_chanop (Window *);
static	char	*status_channel (Window *);
static	char	*status_server (Window *);
static	char	*status_mode (Window *);
static	char	*status_umode (Window *);
static	char	*status_insert_mode (Window *);
static	char	*status_overwrite_mode (Window *);
static	char	*status_away (Window *);
static	char	*status_oper (Window *);
static	char	*status_users (Window *);
static	char	*status_user0s (Window *);
static	char	*status_user1s (Window *);
static	char	*status_user2s (Window *);
static	char	*status_user3s (Window *);
static	char	*status_user4s (Window *);
static	char	*status_user5s (Window *);
static	char	*status_user6s (Window *);
static	char	*status_user7s (Window *);
static	char	*status_user8s (Window *);
static	char	*status_user9s (Window *);
static	char	*status_user10s (Window *);
static	char	*status_user11s (Window *);
static	char	*status_user12s (Window *);
static	char	*status_user13s (Window *);
static	char	*status_user14s (Window *);
static	char	*status_user15s (Window *);
static	char	*status_user16s (Window *);
static	char	*status_user17s (Window *);
static	char	*status_user18s (Window *);
static	char	*status_user19s (Window *);
static	char	*status_user20s (Window *);
static	char	*status_user21s (Window *);
static	char	*status_user22s (Window *);
static	char	*status_user23s (Window *);
static	char	*status_user24s (Window *);
static	char	*status_user25s (Window *);
static	char	*status_user26s (Window *);
static	char	*status_user27s (Window *);
static	char	*status_user28s (Window *);
static	char	*status_user29s (Window *);
static	char	*status_user30s (Window *);
static	char	*status_user31s (Window *);
static	char	*status_user32s (Window *);
static	char	*status_user33s (Window *);
static	char	*status_user34s (Window *);
static	char	*status_user35s (Window *);
static	char	*status_user36s (Window *);
static	char	*status_user37s (Window *);
static	char	*status_user38s (Window *);
static	char	*status_user39s (Window *);
static	char	*status_lag   (Window *);
static	char	*status_dcc (Window *);
static	char	*status_oper_kills (Window *);
static	char	*status_msgcount (Window *);
static	char	*status_hold (Window *);
static	char	*status_version (Window *);
static	char	*status_clock (Window *);
static	char	*status_hold_lines (Window *);
static	char	*status_window (Window *);
static	char	*status_mail (Window *);
static	char	*status_refnum (Window *);
static	char	*status_topic (Window *);
static	char	*status_null_function (Window *);
static	char	*status_notify_windows (Window *);
static	char	*convert_sub_format (const char *, int, char, const char *);
static	char	*status_voice (Window *);
static	char	*status_cpu_saver_mode (Window *);
static	char	*status_dcccount (Window *);
static	char	*status_cdcccount (Window *);
static	char	*status_position (Window *);
static	char	*status_lastjoin (Window *);
static	char	*status_userlist (Window *);
static	char	*status_shitlist (Window *);
static	char	*status_nethack (Window *);
static	char	*status_aop (Window *);
static	char	*status_bitch (Window *);
static	char	*status_newserver (Window *);
static	char	*status_scrollback (Window *);
static	char	*status_percent (Window *);
static	char	*status_windowspec (Window *);
static	char	*status_halfop (Window *);
static	char	*status_notifyusers(Window *);
#define cparse(format, str) convert_output_format(fget_string_var(format), "%s", str)

#if 1
/*
 * This is how we store status line expandos.
 */
struct status_formats {
	int	map;
	char 	key;
	char	*(*callback_function)(Window *);
	int args;	/* number of arguments used by the subformat */
	int	format_set;	/* WSET for subformat, if any */
};


char	*away_format = NULL,
	*hold_lines_format = NULL,
	*channel_format = NULL,
	*notify_format = NULL,
	*cpu_saver_format = NULL,
	*kill_format = NULL,
	*lag_format = NULL,
	*mail_format = NULL,
	*nick_format = NULL,
	*query_format = NULL,
	*server_format = NULL,
	*clock_format = NULL,
	*dcccount_format = NULL,
	*cdcc_format = NULL,
	*msgcount_format = NULL,
	*umode_format = NULL,
	*mode_format = NULL,
	*topic_format = NULL,
	*user_format = NULL;
	
/*
 * This is the list of possible expandos.  Note that you should not use
 * the '{' character, as it would be confusing.  It is already used for 
 * specifying the map.
 */
struct status_formats status_expandos[] = {
{ 0, 'A', status_away,           1,		STATUS_AWAY_WSET },
{ 0, 'B', status_hold_lines,     1,		STATUS_HOLD_LINES_WSET },
{ 0, 'C', status_channel,        1,		STATUS_CHANNEL_WSET },
{ 0, 'D', status_dcc, 	         0, 			-1 },
{ 0, 'E', status_scrollback,     0, 			-1 },
{ 0, 'F', status_notify_windows, 1,		STATUS_NOTIFY_WSET },
{ 0, 'H', status_hold,		 0,			-1 },
{ 0, 'G', status_halfop,	 0,			-1 },
{ 0, 'I', status_insert_mode,    0,			-1 },
{ 0, 'J', status_cpu_saver_mode, 1,		STATUS_CPU_SAVER_WSET },
{ 0, 'K', status_oper_kills,	 2,		STATUS_OPER_KILLS_WSET },
{ 0, 'L', status_lag, 		 1,		STATUS_LAG_WSET },

{ 0, 'M', status_mail,		 1,		STATUS_MAIL_WSET },
{ 0, 'N', status_nickname,	 1,		STATUS_NICKNAME_WSET },
{ 0, 'O', status_overwrite_mode, 0,			-1 },
{ 0, 'P', status_position,       0,			-1 },
{ 0, 'Q', status_query_nick,     1,		STATUS_QUERY_WSET },
{ 0, 'R', status_refnum,         0, 			-1 },
{ 0, 'S', status_server,         1,     	STATUS_SERVER_WSET },
{ 0, 'T', status_clock,          1,      	STATUS_CLOCK_WSET },
{ 0, 'U', status_user0s,		 0, 			-1 },
{ 0, 'V', status_version,	 0, 			-1 },
{ 0, 'W', status_window,	 0, 			-1 },
{ 0, 'X', status_user1s,		 0, 			-1 },
{ 0, 'Y', status_user2s,		 0, 			-1 },
{ 0, 'Z', status_user3s,		 0, 			-1 },
{ 0, '&', status_dcccount,	 2,		STATUS_DCCCOUNT_WSET },
{ 0, '|', status_cdcccount,	 2,		STATUS_CDCCCOUNT_WSET },
{ 0, '^', status_msgcount,	 1,		STATUS_MSGCOUNT_WSET },
{ 0, '#', status_umode,		 1,	     	STATUS_UMODE_WSET },
{ 0, '%', status_percent,	 0, 			-1 },
{ 0, '*', status_oper,		 0, 			-1 },
{ 0, '+', status_mode,		 1,       	STATUS_MODE_WSET },
{ 0, '.', status_windowspec,	 0, 			-1 },
{ 0, '=', status_voice,		 0, 			-1 },
{ 0, '>', status_right_justify,	 0, 			-1 },
{ 0, '-', status_topic,		 1,		STATUS_TOPIC_WSET },
{ 0, '!', status_users,		 5,		STATUS_USERS_WSET },
{ 0, '@', status_chanop,	 0, 			-1 },
{ 0, '0', status_user0s,		 0, 			-1 },
{ 0, '1', status_user1s,		 0, 			-1 },
{ 0, '2', status_user2s,		 0, 			-1 },
{ 0, '3', status_user3s,		 0, 			-1 },
{ 0, '4', status_user4s,		 0, 			-1 },
{ 0, '5', status_user5s,		 0, 			-1 },
{ 0, '6', status_user6s,		 0, 			-1 },
{ 0, '7', status_user7s,		 0, 			-1 },
{ 0, '8', status_user8s,		 0, 			-1 },
{ 0, '9', status_user9s,		 0, 			-1 },
{ 0, 'f', status_shitlist,		 0,			-1 },
{ 0, 'a', status_aop,			 0,			-1 },
{ 0, 'b', status_bitch,			 0,			-1 },
{ 0, 'h', status_nethack,		 0,			-1 },
{ 0, 'l', status_lastjoin,		 0,			-1 },
{ 0, 'n', status_notifyusers,		 0,			-1 },
{ 0, 's', status_newserver,		 0,			-1 },
{ 0, 'u', status_userlist,		 0,			-1 },

{ 1, '0', status_user10s,		0,			-1 },
{ 1, '1', status_user11s,		0,			-1 },
{ 1, '2', status_user12s,		0,			-1 },
{ 1, '3', status_user13s,		0,			-1 },
{ 1, '4', status_user14s,		0,			-1 },
{ 1, '5', status_user15s,		0,			-1 },
{ 1, '6', status_user16s,		0,			-1 },
{ 1, '7', status_user17s,		0,			-1 },
{ 1, '8', status_user18s,		0,			-1 },
{ 1, '9', status_user19s,		0,			-1 },

{ 2, '0', status_user20s,		0,			-1 },
{ 2, '1', status_user21s,		0,			-1 },
{ 2, '2', status_user22s,		0,			-1 },
{ 2, '3', status_user23s,		0,			-1 },
{ 2, '4', status_user24s,		0,			-1 },
{ 2, '5', status_user25s,		0,			-1 },
{ 2, '6', status_user26s,		0,			-1 },
{ 2, '7', status_user27s,		0,			-1 },
{ 2, '8', status_user28s,		0,			-1 },
{ 2, '9', status_user29s,		0,			-1 },

{ 3, '0', status_user30s,		0,			-1 },
{ 3, '1', status_user31s,		0,			-1 },
{ 3, '2', status_user32s,		0,			-1 },
{ 3, '3', status_user33s,		0,			-1 },
{ 3, '4', status_user34s,		0,			-1 },
{ 3, '5', status_user35s,		0,			-1 },
{ 3, '6', status_user36s,		0,			-1 },
{ 3, '7', status_user37s,		0,			-1 },
{ 3, '8', status_user38s,		0,			-1 },
{ 3, '9', status_user39s,		0,			-1 },

};

#define NUMBER_OF_EXPANDOS (sizeof(status_expandos) / sizeof(struct status_formats))
#endif

void *default_status_output_function = make_status;

/*
 * convert_sub_format: This is used to convert the formats of the
 * sub-portions of the status line to a format statement specially designed
 * for that sub-portion.  convert_sub_format looks for occurrences of %c
 * (where c is passed to the function); when found, it is replaced by %s
 * for use in an sprintf.  Only the first 'args' instances are replaced.
 * All other occurrences of % are replaced by %%.  The string returned by 
 * this function must be freed.
 */
static char *convert_sub_format(const char *format, int args, char c, const char *padded)
{
	size_t i = 0;
	char buffer[BIG_BUFFER_SIZE];
	
	if (!format)
		return NULL;

	while (*format && i < sizeof buffer)
	{
		buffer[i++] = *format;

		if (*format == '%')
		{
			format++;

			if (*format == c && args)
			{
				args--;

				if (i < sizeof buffer)
					i += strlcpy(buffer + i, padded, sizeof buffer - i);

				if (i < sizeof buffer)
					buffer[i++] = 's';
			}
			else if (*format == '<')
			{
				const char *saved_format = format;
				size_t saved_i = i;
				format++;

				while (strchr("0123456789.", *format) && i < sizeof buffer)
					buffer[i++] = *format++;
		
				while (*format != '>' && *format)
					format++;
				if (*format)
					format++;

				if (*format == c && args)
				{
					args--;

					if (i < sizeof buffer)
						buffer[i++] = 's';
				}
				else
				{
					i = saved_i;
					format = saved_format;
					
					if (i < sizeof buffer)
						buffer[i++] = '%';
					continue;
				}
			}
			else
			{
				if (i < sizeof buffer)
					buffer[i++] = '%';
				continue;
			}
		}
		format++;
	}	

	if (i > sizeof buffer - 1)
		i = sizeof buffer - 1;
	buffer[i] = 0;

	return m_strdup(buffer);
}

static	char	*convert_format(Window *win, char *format, int k)
{
	char	buffer[BIG_BUFFER_SIZE + 1];
	char	padded[BIG_BUFFER_SIZE + 1];
	int	pos = 0;
	int	*cp;
	int	map;
	char	key;
	int	i;

	cp = &win->func_cnt[k];
	while (*format && pos < BIG_BUFFER_SIZE - 4)
	{
		*padded = 0;
		if (*format != '%')
		{
			buffer[pos++] = *format++;
			continue;
		}

		/* Its a % */
		map = 0;

		/* Find the map, if necessary */
		if (*++format == '{')
		{
			char	*endptr;

			format++;
			map = strtoul(format, &endptr, 10);
			if (*endptr != '}')
			{
				/* Unrecognized */
				continue;
			}
			format = endptr + 1;
		}
		else if (*format == '<')
		{
			size_t pad_len;
	
			format++;
			pad_len = strspn(format, "0123456789.");
			memcpy(padded, format, pad_len);
			padded[pad_len] = 0;

			format += pad_len;
			while(*format && *format != '>') 
				format++;
			if (*format)
				format++;
		}
		key = *format++;

		/* Choke once we get to the maximum number of expandos */
		if (*cp >= MAX_FUNCTIONS)
			continue;

		for (i = 0; i < NUMBER_OF_EXPANDOS; i++)
		{
			char **s = NULL;
			if (status_expandos[i].map != map ||
			    status_expandos[i].key != key)
				continue;

			if (status_expandos[i].format_set != -1)
			{
				s = get_wset_format_var_address(win->wset, status_expandos[i].format_set);
				if (s)
				{
					new_free(s);
					*s = convert_sub_format(
						get_wset_string_var(win->wset, 
							status_expandos[i].format_set), 
						status_expandos[i].args, key, padded);
				}
			}
			buffer[pos++] = '%';
			buffer[pos++] = 's';

			win->status_func[k][(*cp)++] = 
				status_expandos[i].callback_function;
			break;
		}
	}

	win->func_cnt[k] = *cp;
	buffer[pos] = 0;
	return m_strdup(buffer);
}

void BX_build_status(Window *win, char *format, int unused)
{
	int	i, k;
	char	*f = NULL;
	int	ofs = from_server;
	if (!win)
		return;
	if (win->server > -1)
		from_server = win->server;
	for (k = 0; k < 3; k++) 
	{
		if (!win->wset)
			continue;
		new_free(&(win->wset->status_format[k]));
		win->func_cnt[k] = 0;
		switch(k)
		{
			case 0:
				f = win->wset->format_status[1];
				break;
			case 1:
				f = win->wset->format_status[2];
				break;
			case 2:
				f = win->wset->format_status[3];
				break;
		}			

		if (f)
			win->wset->status_format[k] = convert_format(win, f, k);

		for (i = win->func_cnt[k]; i < MAX_FUNCTIONS; i++)
			win->status_func[k][i] = status_null_function;
	}
	update_all_status(win, NULL, 0);
	from_server = ofs;
}

/* Take a built status line and justify it to window size, handling the >% right-justify status format. */
static void justify_status(Window *win, char *buffer, size_t buffer_size)
{
	char padding[BIG_BUFFER_SIZE + 1];
	char *str;
	char *rhs = NULL;
	char *ptr;
	int printable = 0;
	const char *padchar = " ";
	size_t padchar_len = 1;

	/*
	 * This converts away any ansi codes in the status line
	 * in accordance with the current settings.  This leaves us
	 * with nothing but logical characters, which are then easy
	 * to count. :-)
	 */
	str = strip_ansi(buffer);

	/*
	 * Count out the characters.
	 * Walk the entire string, looking for nonprintable
	 * characters.  We count all the printable characters
	 * on both sides of the %> tag.
	 */
	ptr = str;

	while (*ptr)
	{
		/*
		 * The FIRST such %> tag is used.
		 * Using multiple %>s is bogus.
		 */
		if (*ptr == '\f' && !rhs)
		{
			*ptr++ = 0;
			rhs = ptr;
		}

		/*
		 * Skip over color attributes if we're not
		 * doing color.
		 */
		else if (*ptr == COLOR_CHAR)
		{
			ptr = skip_ctl_c_seq(ptr, NULL, NULL, 0);
		}

		/*
		 * If we have a ROM character code here, we need to
		 * copy it to the output buffer, as well as the fill
		 * character, and increment the printable counter by
		 * only 1.
		 */
		else if (*ptr == ROM_CHAR)
		{
			if (!rhs)
			{
				padchar = ptr;
				padchar_len = 4;
			}
			ptr += 4;
			printable += 1;
		}

		/*
		 * Is it NOT a printable character?
		 */
		else if ((*ptr == REV_TOG) || (*ptr == UND_TOG) ||
				(*ptr == ALL_OFF) || (*ptr == BOLD_TOG) ||
				(*ptr == BLINK_TOG))
			ptr++;
		/*
		 * So it is a printable character.
		 * Or maybe its a tab. ;-)
		 */
		else
		{
			if (!rhs)
			{
				padchar = ptr;
				padchar_len = 1;
			}
			ptr += 1;
			printable += 1;
		}

		/*
		 * Don't allow more than CO printable characters
		 */
		if (printable >= win->screen->co)
		{
			*ptr = 0;
			break;
		}
	}

	/* What will we be filling with? */
	if (get_int_var(STATUS_NO_REPEAT_VAR))
	{
		padchar = " ";
		padchar_len = 1;
	}

	/*
	 * Now if we have a rhs, or the user wants padding anyway, then we have to pad it.
	 */
	padding[0] = 0;
	if (rhs || get_int_var(FULL_STATUS_LINE_VAR))
	{
		int chars = win->screen->co - printable - 1;
		char * const endptr = &padding[sizeof padding - padchar_len];

		ptr = padding;
		while (chars-- >= 0 && ptr < endptr)
		{
			memcpy(ptr, padchar, padchar_len);
			ptr += padchar_len;
		}
		*ptr = 0;
	}

	snprintf(buffer, buffer_size, "%s%s%s%s", str, padding, rhs ? rhs : "", ALL_OFF_STR);
	new_free(&str);
}

void make_status(Window *win)
{
	char buffer[BIG_BUFFER_SIZE + 1];
	char *func_value[MAX_FUNCTIONS+10] = {NULL};
	
	int	len = 1,
		status_line,
		ansi = get_int_var(DISPLAY_ANSI_VAR);

	/* The second status line is only displayed in the bottom window
	 * and should always be displayed, no matter what SHOW_STATUS_ALL
	 * is set to - krys
	 */
	for (status_line = 0 ; status_line < 1+win->double_status + win->status_lines; status_line++)
	{
		char *str;
		int line = status_line;
		int i;

		if (!win->wset || !win->wset->status_format[line])
			continue;
			
		for (i = 0; i < MAX_FUNCTIONS; i++)
			func_value[i] = (win->status_func[line][i]) (win);
		len = 1;
		
		if (get_int_var(REVERSE_STATUS_VAR))
			buffer[0] = get_int_var(REVERSE_STATUS_VAR) ? REV_TOG : ' ';
		else
			len = 0;
		str = &buffer[len];                                        
		snprintf(str, BIG_BUFFER_SIZE - 1, 
			win->wset->status_format[line],
			func_value[0], func_value[1], func_value[2],
			func_value[3], func_value[4], func_value[5],
			func_value[6], func_value[7], func_value[8],
			func_value[9], func_value[10], func_value[11],
			func_value[12], func_value[13], func_value[14],
			func_value[15], func_value[16], func_value[17],
			func_value[18], func_value[19], func_value[20],
			func_value[21], func_value[22], func_value[23],
			func_value[24], func_value[25], func_value[26],
			func_value[27], func_value[28], func_value[29],
			func_value[30], func_value[31],func_value[32],
			func_value[33], func_value[34],func_value[35],
			func_value[36], func_value[37],func_value[38]);

			/*  Patched 26-Mar-93 by Aiken
			 *  make_window now right-justifies everything 
			 *  after a %>
			 *  it's also more efficient.
			 */

		/*
		 * If the user wants us to, pass the status bar through the
		 * expander to pick any variables/function calls.
		 * This is horribly expensive, but what do i care if you
		 * want to waste cpu cycles? ;-)
		 */
		if (get_int_var(STATUS_DOES_EXPANDOS_VAR))
		{
			int  af = 0;

			str = expand_alias(buffer, empty_string, &af, NULL);
			strlcpy(buffer, str, sizeof buffer);
			new_free(&str);
		}

		justify_status(win, buffer, sizeof buffer);

		do_hook(STATUS_UPDATE_LIST, "%d %d %s", 
			win->refnum, 
			status_line, 
			buffer);
		if (dumb_mode)
		{
			/* lets see what happens here */
			continue;
		}
		
		if (!win->wset->status_line[status_line] ||
			strcmp(buffer, win->wset->status_line[status_line]))

		{
			char *st = NULL;
			malloc_strcpy(&win->wset->status_line[status_line], buffer);
			output_screen = win->screen;
			st = cparse((line==3)?FORMAT_STATUS3_FSET:(line==2)?FORMAT_STATUS2_FSET:(line==1)?FORMAT_STATUS1_FSET:FORMAT_STATUS_FSET, buffer);
			if (!ansi)
				st = stripansicodes(st);
			if (win->status_lines && (line == win->double_status+win->status_lines) && win->status_split)
				term_move_cursor(0,win->top);
			else if (win->status_lines && !win->status_split)
				term_move_cursor(0,win->bottom+status_line-win->status_lines);
			else
				term_move_cursor(0,win->bottom+status_line);

			output_line(st);
			cursor_in_display(win);
			term_bold_off();
		} 
	}
	cursor_to_input();
}

/* Formats a status line in the context of the current window, 
 * for the $statsparse() scripting function. */
char *stat_convert_format(Window *win, char *form)
{
	int map, key, i, pos = 0;
	char *ptr = form;
	char buffer[2 * BIG_BUFFER_SIZE + 1];

	if (!form || !*form)
		return m_strdup(empty_string);	

	*buffer = 0;
	while (*ptr && pos < (2 * BIG_BUFFER_SIZE) - 4)
	{
		if (*ptr != '%')
		{
			buffer[pos++] = *ptr++;
			continue;
		}

		/* Its a % */
		map = 0;

		/* Find the map, if necessary */
		if (*++ptr == '{')
		{
			char	*endptr;

			ptr++;
			map = strtoul(ptr, &endptr, 10);
			if (*endptr != '}')
			{
				/* Unrecognized */
				continue;
			}
			ptr = endptr + 1;
		}

		key = *ptr++;

		/* Choke once we get to the maximum number of expandos */
		for (i = 0; i < NUMBER_OF_EXPANDOS; i++)
		{
			if (status_expandos[i].map != map || status_expandos[i].key != key)
				continue;
			buffer[pos] = 0;
			strlcat(buffer, status_expandos[i].callback_function(win), sizeof buffer);
			pos = strlen(buffer);
			break;
		}
	}

	buffer[pos] = 0;

	if (get_int_var(STATUS_DOES_EXPANDOS_VAR))
	{
		int  af = 0;
		char *stuff;
		Window *old = current_window;

		current_window = win;
		stuff = expand_alias(buffer, empty_string, &af, NULL);
		strlcpy(buffer, stuff, sizeof buffer);
		new_free(&stuff);
		current_window = old;
	}

	justify_status(win, buffer, sizeof buffer);
	return m_strdup(buffer);
}

/* Some useful macros */
/*
 * This is used to get the current window on a window's screen
 */
#define CURRENT_WINDOW window->screen->current_window

/*
 * This tests to see if the window IS the current window on its screen
 */
#define IS_CURRENT_WINDOW (window->screen->current_window == window)

/*
 * This tests to see if all expandoes are to appear in all status bars
 */
#define SHOW_ALL_WINDOWS (get_int_var(SHOW_STATUS_ALL_VAR))

/*
 * "Current-type" window expandoes occur only on the current window for a 
 * screen.  However, if /set show_status_all is on, then ALL windows act as
 * "Current-type" windows.
 */
#define DISPLAY_ON_WINDOW (IS_CURRENT_WINDOW || SHOW_ALL_WINDOWS)

#define RETURN_EMPTY  return empty_string


static	char	*status_nickname(Window *window)
{
static char my_buffer[MY_BUFFER/2+1];
	snprintf(my_buffer, MY_BUFFER/2, window->wset->nick_format, get_server_nickname(window->server));
	return my_buffer;
}

static	char	*status_server(Window *window)
{
	char	*name;
static	char	my_buffer[MY_BUFFER+1];
	if (connected_to_server)
	{
		if (window->server != -1)
		{
			if (window->wset->server_format)
			{
				if (!(name = get_server_itsname(window->server)))
					name = get_server_name(window->server);
				snprintf(my_buffer, MY_BUFFER, window->wset->server_format, name);
			}
			else
				RETURN_EMPTY;
		}
		else
			strcpy(my_buffer, " No Server");
	}
	else
		RETURN_EMPTY;
	return my_buffer;
}

static	char	*status_query_nick(Window *window)
{
static	char my_buffer[BIG_BUFFER_SIZE+1];

	if (window->query_nick && window->wset->query_format)
	{
		snprintf(my_buffer, BIG_BUFFER_SIZE, window->wset->query_format, window->query_nick);
		return my_buffer;
	}
	else
		RETURN_EMPTY;
}

static	char	*status_right_justify(Window *window)
{
static	char	my_buffer[] = "\f";

	return my_buffer;
}

static	char	*status_notify_windows(Window *window)
{
	int doneone = 0;
	char notes[MY_BUFFER + 2];
	static char my_buffer[MY_BUFFER / 2 + 1];

	if (get_int_var(SHOW_STATUS_ALL_VAR) || window == window->screen->current_window)
	{
		Window *notify_win = NULL;
		*notes = 0;
		while ((traverse_all_windows(&notify_win)))
		{
			if (notify_win->miscflags & WINDOW_NOTIFIED)
			{
				if (doneone++)
					strlcat(notes, ",", sizeof notes);
				strlcat(notes, ltoa(notify_win->refnum), sizeof notes);
			}
		}
	}
	if (doneone && window->wset->notify_format)
	{
		snprintf(my_buffer, sizeof my_buffer, window->wset->notify_format, notes);
		return my_buffer;
	}
	RETURN_EMPTY;
}

static	char	*status_clock(Window *window)
{
static	char	my_buf[MY_BUFFER+1];

	if ((get_int_var(CLOCK_VAR) && window->wset->clock_format)  &&
	    (get_int_var(SHOW_STATUS_ALL_VAR) ||
	    (window == window->screen->current_window)))
		snprintf(my_buf, MY_BUFFER, window->wset->clock_format, update_clock(GET_TIME));
	else
		RETURN_EMPTY;
	return my_buf;
}

static	char	*status_mode(Window *window)
{
char		*mode = NULL;
static  char	my_buffer[MY_BUFFER+1];
	if (window->current_channel)
	{
		mode = get_channel_mode(window->current_channel,window->server);
		if (mode && *mode && window->wset->mode_format)
		{
			if (get_int_var(STATUS_DOES_EXPANDOS_VAR))
			{
				char *mode2 = alloca(strlen(mode) * 2 + 1);
				double_quote(mode, "$", mode2);
				mode = mode2;
			}
			snprintf(my_buffer, MY_BUFFER, window->wset->mode_format, mode);
		}
		else
			RETURN_EMPTY;
	} else
		RETURN_EMPTY;
	return my_buffer;
}


static	char	*status_umode(Window *window)
{
	char	localbuf[MY_BUFFER+1];
static char my_buffer[MY_BUFFER/2+1];

	if ((connected_to_server) && !get_int_var(SHOW_STATUS_ALL_VAR)
	    && (window->screen->current_window != window))
		*localbuf = 0;
	else {
		if (window->server > -1)
			strlcpy(localbuf, get_umode(window->server), sizeof localbuf);
		else
			*localbuf = 0;
	}
	
	if (*localbuf && window->wset->umode_format)
		snprintf(my_buffer, MY_BUFFER/2, window->wset->umode_format, localbuf);
	else
		RETURN_EMPTY;
	return my_buffer;
}

static	char	*status_chanop(Window *window)
{
    char	*text;

	if (window->current_channel && 
        get_channel_oper(window->current_channel, window->server) && 
        (text = get_wset_string_var(window->wset, STATUS_CHANOP_WSET)))
        return text;

    RETURN_EMPTY;
}
static	char	*status_halfop(Window *window)
{
    char	*text;

    if (window->current_channel && 
        get_channel_halfop(window->current_channel, window->server) &&
        (text = get_wset_string_var(window->wset, STATUS_HALFOP_WSET)))
        return text;

    RETURN_EMPTY;
}


static	char	*status_hold_lines(Window *window)
{
	int	num;
static  char	my_buffer[MY_BUFFER/2+1];
	
	if ((num = (window->lines_held /10) * 10))
	{
		snprintf(my_buffer, MY_BUFFER/2, window->wset->hold_lines_format, ltoa(num));
		return(my_buffer);
	}
	RETURN_EMPTY;
}

static	char	*status_msgcount(Window *window)
{
static  char	my_buffer[MY_BUFFER/2+1];

	if (get_int_var(MSGCOUNT_VAR) && window->wset->msgcount_format)
	{
		snprintf(my_buffer, MY_BUFFER/2, window->wset->msgcount_format, ltoa(get_int_var(MSGCOUNT_VAR)));
		return my_buffer;
	}
	RETURN_EMPTY;
}

static	char	*status_channel(Window *window)
{
	char	channel[IRCD_BUFFER_SIZE + 1];
static	char	my_buffer[IRCD_BUFFER_SIZE + 1];

	if (window->current_channel/* && chan_is_connected(s, window->server)*/)
	{
		int num;
		if (get_int_var(HIDE_PRIVATE_CHANNELS_VAR) &&
		    is_channel_mode(window->current_channel,
				MODE_PRIVATE | MODE_SECRET,
				window->server))
			strlcpy(channel, "*private*", sizeof channel);
		else
			strlcpy(channel, window->current_channel, sizeof channel);

		#ifdef WANT_HEBREW
		if (get_int_var(HEBREW_TOGGLE_VAR))
			hebrew_process((char *)&channel);
		#endif

		if ((num = get_int_var(CHANNEL_NAME_WIDTH_VAR)) &&
		    ((int) strlen(channel) > num))
			channel[num] = (char) 0;
		snprintf(my_buffer, IRCD_BUFFER_SIZE, window->wset->channel_format, channel);
		return my_buffer;
	}
	RETURN_EMPTY;
}

static	char	*status_mail(Window *window)
{
	char	*number;
static	char	my_buffer[MY_BUFFER/2+1];

	if (window && (get_int_var(MAIL_VAR) && (number = check_mail()) && window->wset->mail_format) &&
	    (get_int_var(SHOW_STATUS_ALL_VAR) ||
	    (window == window->screen->current_window)))
	{
		snprintf(my_buffer, MY_BUFFER/2, window->wset->mail_format, number);
		return my_buffer;
	}
	RETURN_EMPTY;
}

static	char	*status_insert_mode(Window *window)
{
char	*text;

	if (get_int_var(INSERT_MODE_VAR) && (get_int_var(SHOW_STATUS_ALL_VAR)
	    || (window->screen->current_window == window)))
		if ((text = get_string_var(STATUS_INSERT_VAR)))
			return text;
	RETURN_EMPTY;
}

static	char	*status_overwrite_mode(Window *window)
{
char	*text;

	if (!get_int_var(INSERT_MODE_VAR) && (get_int_var(SHOW_STATUS_ALL_VAR)
	    || (window->screen->current_window == window)))
	{
	    if ((text = get_string_var(STATUS_OVERWRITE_VAR)))
		return text;
	}
	RETURN_EMPTY;
}

static	char	*status_away(Window *window)
{
static char	my_buffer[MY_BUFFER+1];

	if (window && connected_to_server && !get_int_var(SHOW_STATUS_ALL_VAR)
	    && (window->screen->current_window != window))
		RETURN_EMPTY;
	else if (window)
	{
		if (window->server != -1 && get_server_away(window->server))
		{
			snprintf(my_buffer, MY_BUFFER, window->wset->away_format, ltoa(get_int_var(MSGCOUNT_VAR)));
			return my_buffer;
		}
		else
			RETURN_EMPTY;
	}
	RETURN_EMPTY;
}


/*
 * This is a generic status_userX variable.  All of these go to the
 * current-type window, although i think they should go to all windows.
 */
#define STATUS_VAR(x) \
static	char	*status_user ## x ## s (Window *window)			\
{									\
	char	*text;							\
									\
	if ((text = get_wset_string_var(window->wset, STATUS_USER ## x ## _WSET)) && 	\
	     DISPLAY_ON_WINDOW)						\
		return text;						\
	else								\
		RETURN_EMPTY;					\
}

STATUS_VAR(0)
STATUS_VAR(1)
STATUS_VAR(2)
STATUS_VAR(3)
STATUS_VAR(4)
STATUS_VAR(5)
STATUS_VAR(6)
STATUS_VAR(7)
STATUS_VAR(8)
STATUS_VAR(9)
STATUS_VAR(10)
STATUS_VAR(11)
STATUS_VAR(12)
STATUS_VAR(13)
STATUS_VAR(14)
STATUS_VAR(15)
STATUS_VAR(16)
STATUS_VAR(17)
STATUS_VAR(18)
STATUS_VAR(19)
STATUS_VAR(20)
STATUS_VAR(21)
STATUS_VAR(22)
STATUS_VAR(23)
STATUS_VAR(24)
STATUS_VAR(25)
STATUS_VAR(26)
STATUS_VAR(27)
STATUS_VAR(28)
STATUS_VAR(29)
STATUS_VAR(30)
STATUS_VAR(31)
STATUS_VAR(32)
STATUS_VAR(33)
STATUS_VAR(34)
STATUS_VAR(35)
STATUS_VAR(36)
STATUS_VAR(37)
STATUS_VAR(38)
STATUS_VAR(39)

static	char	*status_hold(Window *window)
{
char	*text;

	if (window->holding_something && (text = get_wset_string_var(window->wset, STATUS_HOLD_WSET)))
		return(text);
	RETURN_EMPTY;
}

static	char	*status_lag (Window *window)
{
static  char	my_buffer[MY_BUFFER/2+1];
	if ((window->server > -1) && window->wset->lag_format)
	{
		if (get_server_lag(window->server) > -1)
		{
			char p[40];
			sprintf(p, "%2d",get_server_lag(window->server)); 
			snprintf(my_buffer,MY_BUFFER/2, window->wset->lag_format, p);
		}
		else
			snprintf(my_buffer, MY_BUFFER/2, window->wset->lag_format, "??");
		return(my_buffer);
	}
	RETURN_EMPTY;
}

static	char	*status_topic (Window *window)
{
	static char my_buffer[IRCD_BUFFER_SIZE];

	if (window && window->current_channel && window->wset->topic_format)
	{
		ChannelList *chan;
		if ((chan = lookup_channel(window->current_channel, window->server, 0)))
		{
			char *t;
			if ((t = chan->topic))
			{
				char *t2 = alloca(strlen(t) * 2 + 1 );
				if (get_int_var(STATUS_DOES_EXPANDOS_VAR))
					double_quote(t, "()[]$\"", t2);
				else
					strcpy(t2, t);
				snprintf(my_buffer, sizeof my_buffer, window->wset->topic_format, stripansicodes(t2));
			}
			else
				strlcpy(my_buffer, "No Topic", sizeof my_buffer);
			return my_buffer;
		}
	}
	RETURN_EMPTY;
}

static	char	*status_oper(Window *window)
{
char	*text;

	if (get_server_operator(window->server) &&
			(text = get_string_var(STATUS_OPER_VAR)) &&
			(get_int_var(SHOW_STATUS_ALL_VAR) ||
			connected_to_server || 
			(window->screen->current_window == window)))
		return(text);
	RETURN_EMPTY;
}

static	char	*status_voice(Window *window)
{
char	*text;
	if (window->current_channel &&
	    get_channel_voice(window->current_channel, window->server) &&
	    (text = get_wset_string_var(window->wset, STATUS_VOICE_WSET)))
		return text;
	RETURN_EMPTY;
}

static	char	*status_window(Window *window)
{
char	*text;
	if ((number_of_windows_on_screen(window) > 1) && (window->screen->current_window == window) &&
	    (text = get_wset_string_var(window->wset, STATUS_WINDOW_WSET)))
		return(text);
	RETURN_EMPTY;
}

static	char	*status_refnum(Window *window)
{
static char my_buffer[MY_BUFFER/3+1];
	strlcpy(my_buffer, window->name ? window->name : ltoa(window->refnum), sizeof my_buffer);
	return (my_buffer);
}

static	char	*status_version(Window *window)
{
	if ((connected_to_server) && !get_int_var(SHOW_STATUS_ALL_VAR)
	    && (window->screen->current_window != window))
		return(empty_string);
	return ((char *)irc_version);
}

static	char	* status_oper_kills (Window *window)
{
static char my_buffer[MY_BUFFER+1];
	if (window->wset->kills_format && (nick_collisions || oper_kills))
	{
		char tmp[30];
		snprintf(tmp, 29, "%ld", nick_collisions); 
		snprintf(my_buffer, MY_BUFFER, window->wset->kills_format, tmp, ltoa(oper_kills));
		return my_buffer;
	}
	RETURN_EMPTY;	
}

static char *status_dcccount (Window *window)
{
extern int get_count_stat, send_count_stat;
static char my_buffer[2 * MY_BUFFER+1];
	if (window->wset->dcccount_format && send_count_stat)
	{
		char tmp[30];
		strcpy(tmp, ltoa(send_count_stat));
		snprintf(my_buffer, 2 * MY_BUFFER, window->wset->dcccount_format, ltoa(get_count_stat), tmp);
		return my_buffer;
	}
	RETURN_EMPTY;
}

static char *status_cdcccount (Window *window)
{
#ifdef WANT_CDCC
extern int cdcc_numpacks, send_numpacks;
static char my_buffer[2 * MY_BUFFER+1];
	if (window->wset->cdcc_format && cdcc_numpacks)
	{
		char tmp[30];
		strcpy(tmp, ltoa(cdcc_numpacks));
		snprintf(my_buffer, 2 * MY_BUFFER, window->wset->cdcc_format, ltoa(send_numpacks), tmp);
		return my_buffer;
	}
#endif
	RETURN_EMPTY;
}

static char *status_cpu_saver_mode (Window *window)
{
static char my_buffer[MY_BUFFER/2+1];
	if (cpu_saver && window->wset->cpu_saver_format)
	{
		snprintf(my_buffer, MY_BUFFER/2, window->wset->cpu_saver_format, "cpu");
		return my_buffer;
	}

	RETURN_EMPTY;
}

static	char	* status_users (Window *window)
{
static char my_buffer[MY_BUFFER * 2 + 1];
ChannelList *chan;
NickList *nick;
int serv = window->server;
	if (window->server != -1 && window->wset->status_users_format)
	{
		if ((chan = prepare_command(&serv, NULL, PC_SILENT)))
		{
			int ops = 0, nonops = 0, voice = 0, ircop = 0, friends = 0;
			char buff[40], buff1[40], buff2[40], buff3[40], buff4[40];
			 
			for (nick = next_nicklist(chan, NULL); nick; nick = next_nicklist(chan, nick))
			{
				if (nick_isop(nick))
					ops++;
				else
					nonops++;
				if (nick_isvoice(nick))
					voice++;
				if (nick_isircop(nick))
					ircop++;
				if (nick->userlist)
					friends++;
			}
			strcpy(buff, ltoa(ops)); 
			strcpy(buff1, ltoa(nonops));
			strcpy(buff2,ltoa(ircop)); 
			strcpy(buff3, ltoa(voice));
			strcpy(buff4, ltoa(friends));
			snprintf(my_buffer, MY_BUFFER*2, window->wset->status_users_format, buff, buff1, buff2, buff3, buff4);
			return my_buffer;
		}
	}
	RETURN_EMPTY;	
}

static	char	*status_null_function(Window *window)
{
	RETURN_EMPTY;
}


static char *status_dcc(Window *window)
{
	if (window->window_level & LOG_DCC || !window->window_level)
		return(DCC_get_current_transfer());
	RETURN_EMPTY;
}

static char *status_position (Window *window)
{
static char my_buffer[MY_BUFFER/2+1];

	snprintf(my_buffer, MY_BUFFER/2, "(%d-%d)", window->lines_scrolled_back,
					window->distance_from_display);
	return my_buffer;
}
 
static	char	*status_userlist (Window *window)
{
ChannelList *chan;
int serv = window->server;
static char my_buffer[3] = "\0";
	if (window->server != -1)
	{
		if ((chan = prepare_command(&serv, NULL, PC_SILENT)))
		{
			my_buffer[0] = chan->csets->set_userlist ? 'U':'u';
			return my_buffer;
		}
	}
	RETURN_EMPTY;
}

static	char	*status_shitlist (Window *window)
{
ChannelList *chan;
int serv = window->server;
static char my_buffer[3] = "\0";
	if (window->server != -1)
	{
		if ((chan = prepare_command(&serv, NULL, PC_SILENT)))
		{
			my_buffer[0] = chan->csets->set_shitlist ? 'S':'s';
			return my_buffer;
		}
	}
	RETURN_EMPTY;
}

static	char	*status_nethack (Window *window)
{
ChannelList *chan;
int serv = window->server;
static char my_buffer[3] = "\0";
	if (window->server != -1)
	{
		if ((chan = prepare_command(&serv, NULL, PC_SILENT)))
		{
			my_buffer[0] = chan->csets->set_hacking ? 'H':'h';
			return my_buffer;
		}
	}
	RETURN_EMPTY;
}

static	char	*status_aop (Window *window)
{
ChannelList *chan;
int serv = window->server;
static char my_buffer[3] = "\0";
	if (window->server != -1)
	{
		if ((chan = prepare_command(&serv, NULL, PC_SILENT)))
		{
			my_buffer[0] = chan->csets->set_aop ? 'A':'a';
			return my_buffer;
		}
	}
	RETURN_EMPTY;
}

static	char	*status_bitch (Window *window)
{
ChannelList *chan;
int serv = window->server;
static char my_buffer[3] = "\0";
	if (window->server != -1)
	{
		if ((chan = prepare_command(&serv, NULL, PC_SILENT)))
		{
			my_buffer[0] = chan->csets->bitch_mode ? 'B':'b';
			return my_buffer;
		}
	}
	RETURN_EMPTY;
}

static char *status_notifyusers (Window *window)
{
int serv = window->server;
static char my_buffer[40] = "\0";
int on, off;
	notify_count(serv, &on, &off);
	sprintf(my_buffer, "%d %d", on, off);
	return my_buffer;
}

static char *status_lastjoin (Window *window)
{
	if (window->server != -1)
	{
		if (joined_nick)
			return joined_nick;
	}
	RETURN_EMPTY;
}

static char *status_newserver (Window *window)
{
	if (window->server != -1)	
		return ov_server(window->server);
	RETURN_EMPTY;
}

static char *status_scrollback(Window *win)
{
char *stuff;
	if (win->scrollback_point &&
	    (stuff = get_wset_string_var(win->wset, STATUS_SCROLLBACK_WSET)))
		return stuff;
	else
		RETURN_EMPTY;
}

static	char	*status_windowspec	(Window *window)
{
static char my_buffer[81];
	if (window->wset->window_special_format)
		strlcpy(my_buffer, window->wset->window_special_format, sizeof my_buffer);
	else
		*my_buffer = 0;

	return my_buffer;
}

static	char	*status_percent		(Window *window)
{
	static	char	percent[] = "%";
	return	percent;
}

