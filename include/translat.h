/*
 * Global stuff for translation tables.
 *
 * Tomten, tomten@solace.hsh.se / tomten@lysator.liu.se
 *
 * @(#)$Id$
 */
#ifndef TRANSLAT_H_
#define TRANSLAT_H_

	void	set_translation (Window *, char *, int);
	void	enter_digraph (char, char *);
	unsigned char	get_digraph (unsigned char);
	void	digraph (char *, char *, char *, char *);
	void	save_digraphs (FILE *);

extern	unsigned char	transToClient[256];
extern	unsigned char	transFromClient[256];
extern	char	translation;

#define DIG_TABLE_SIZE 256
extern	unsigned char	dig_table_lo[DIG_TABLE_SIZE];
extern	unsigned char	dig_table_hi[DIG_TABLE_SIZE];
extern	unsigned char	dig_table_di[DIG_TABLE_SIZE];

extern	char	digraph_hit;
extern	unsigned char	digraph_first;

#endif /* TRANSLAT_H_ */
