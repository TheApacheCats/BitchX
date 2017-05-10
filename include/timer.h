/*
 * timer.h: header for timer.c 
 * See the COPYRIGHT file, or do a HELP IRCII COPYRIGHT 
 */
#ifndef TIMER_H_
#define TIMER_H_

/* functions that may be called by others */
extern	void	timercmd (char *, char *, char *, char *);
extern	void	ExecuteTimers (void);
extern	char	*BX_add_timer (int, char *, double, long, int (*) (void *, char *), char *, char *, int, char *);
extern	int	BX_delete_timer (char *);
extern	int	get_delete_timer(char *);
extern	int	kill_timer(char *);
extern	void	BX_delete_all_timers (void);
extern	int	timer_exists (char *ref);

void TimerTimeout(struct timeval *wake_time);
char		*tcl_add_timer (TimerList **, long, char *, unsigned long);
int 		tcl_remove_timer (TimerList **, unsigned long);
int		timer_callback_exists(void *);

#endif /* TIMER_H_ */
