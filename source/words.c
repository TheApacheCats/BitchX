/*
 * words.c -- right now it just holds the stuff i wrote to replace
 * that beastie arg_number().  Eventually, i may move all of the
 * word functions out of ircaux and into here.  Now wouldnt that
 * be a beastie of a patch! Beastie! Beastie!
 *
 * Oh yea.  This file is beastierighted (C) 1994 by the beastie author.
 * Right now the only author is Jeremy "Beastie" Nelson.  See the
 * beastieright file for beastie info.
 */

#include "irc.h"
static char cvsrevision[] = "$Id$";
CVS_REVISION(words_c)
#include "ircaux.h"
#include "modval.h"

/* strsearch()
 *
 * If how > 0, returns a pointer to the how'th matching character forwards
 * from mark, in the string starting at start.
 * If how < 0, returns a pointer to the -how'th matching character backwards
 * from mark, in the string starting at start.
 * If how == 0, returns NULL.
 *
 * NULL mark begins the search at start.
 *
 * A matching character is any character in chars, unless chars starts with ^, 
 * in which case a matching character is any character NOT in chars.
 *
 * If there are insufficient matching characters, NULL is returned.
 */
extern char	*BX_strsearch(const char *start, const char *mark, const char *chars, int how)
{
	const char *ptr = NULL;

	if (!mark)
		mark = start;

	if (how > 0)   /* forward search */
	{
		for (; how > 0 && mark; how--)
		{
			ptr = sindex(mark, chars);
			if (ptr)
				mark = ptr + 1;
			else
				mark = NULL;
		}
	}
	else if (how < 0)
	{
		ptr = rsindex(mark, start, chars, -how);
	}

	return (char *)ptr;
}

/* move_to_word()
 *
 * Return a pointer to the first character of the Nth word in a string.
 * The first word is always numbered zero.
 */
extern char	*BX_move_to_word(const char *start, int word)
{
	const char *pointer = start;

	/* This fixes a bug that counted leading spaces as
	 * a word, when they're really not a word.... 
	 * (found by Genesis K.)
	 *
	 * The stock client strips leading spaces on both
	 * the cases $0 and $-0.  I personally think this
	 * is not the best choice, but I'm not going to stick
	 * my foot in this one... I'm just going to go with
	 * what the stock client does...
	 */
	while (*pointer && my_isspace(*pointer))
		pointer++;

	for (; word > 0 && *pointer; word--)
	{
		while (*pointer && !my_isspace(*pointer))
			pointer++;
		while (*pointer && my_isspace(*pointer))
			pointer++;
	}

	return (char *)pointer;
}

/* move_word_rel()
 *
 * Take a string and return a pointer offset a number of words relative to a given mark.
 * Positive offset N returns a pointer to Nth next word (not counting the current word,
 * if the mark is within a word).  Negative offset -N returns a pointer to the Nth
 * previous word, counting the current word.  Offset 0 leaves the mark unchanged.
 */
extern char	*BX_move_word_rel (const char *start, char **mark, int word)
{
	const char *pointer = *mark;

	if (!*start) 	/* null string, return it */
		return (char *)start;

	if (word >= 0)
	{
		for (;word > 0 && *pointer;word--)
		{
			/* Move pointer to first space after current word */
			while (*pointer && !my_isspace(*pointer))
				pointer++;
			/* Move pointer to first character of next word */
			while (*pointer && my_isspace(*pointer)) 
				pointer++;
		}
	}
	else /* word < 0 */
	{
		/* If we are in between words, find the previous word */
		while (pointer > start && my_isspace(pointer[0]))
			pointer--;
		/* Move pointer to first character of current word */
		while (pointer > start && !my_isspace(pointer[-1]))
			pointer--;

		for (word++; word < 0 && pointer > start; word++)
		{
			/* Move pointer to first space after previous word. */
			while (pointer > start && my_isspace(pointer[-1]))
				pointer--;

			/* Move pointer to first character of word */
			while (pointer > start && !my_isspace(pointer[-1]))
				pointer--;
		}
	}

	return *mark = (char *)pointer;
}

/*
 * extract2 is the word extractor that is used when its important to us
 * that 'firstword' get special treatment if it is negative (specifically,
 * that it refer to the "firstword"th word from the END).  This is used
 * basically by the ${n}{-m} expandos and by function_rightw(). 
 *
 * Note that because of a lot of flak, if you do an expando that is
 * a "range" of words, unless you #define STRIP_EXTRANEOUS_SPACES,
 * the "n"th word will be backed up to the first character after the
 * first space after the "n-1"th word.  That apparently is what everyone
 * wants, so that will be the default.  Those of us who may not like
 * that behavior or are at ambivalent can just #define it.
 */
#undef STRIP_EXTRANEOUS_SPACES
extern char	*BX_extract2(const char *start, int firstword, int lastword)
{
	/* If firstword or lastword is negative, then
	   we take those values from the end of the string */
	char *mark;
	char *mark2;
	char *booya = NULL;

	/* If firstword is EOS, then the user wants the last word */
	if (firstword == EOS)
	{
		mark = (char *)start + strlen(start);
		mark = move_word_rel(start, &mark, -1);
#ifndef NO_CHEATING
		/* 
		 * Really. the only case where firstword == EOS is
		 * when the user wants $~, in which case we really
		 * don't need to do all the following crud.  Of
		 * course, if there ever comes a time that the
		 * user would want to start from the EOS (when??)
		 * we couldnt make this assumption.
		 */
		return m_strdup(mark);
#endif
	}

	/* SOS is used when the user does $-n, all leading spaces 
	 * are retained  
	 */
	else if (firstword == SOS)
		mark = (char *)start;

	/* If the firstword is positive, move to that word */
	else if (firstword >= 0)
	{
		mark = move_to_word(start, firstword);
		if (!*mark)
			return m_strdup(empty_string);
	}
	/* Otherwise, move to the firstwords from the end */
	else
	{
		mark = (char *)start + strlen((char *)start);
		move_word_rel(start, &mark, firstword);
	}

#ifndef STRIP_EXTRANEOUS_SPACES
	/* IF the user did something like this:
	 *	$n-  $n-m
	 * then include any leading spaces on the 'n'th word.
	 * this is the "old" behavior that we are attempting
	 * to emulate here.
	 */
#ifndef NO_CHEATING
	if (lastword == EOS || (lastword > firstword))
#else
	if (((lastword == EOS) && (firstword != EOS)) || (lastword > firstword))
#endif
	{
		while (mark > start && my_isspace(mark[-1]))
			mark--;
		if (mark > start)
			mark++;
	}
#endif

	/* 
	 * When we find the last word, we need to move to the 
         * END of the word, so that word 3 to 3, would include
	 * all of word 3, so we sindex to the space after the word
	 */
	if (lastword == EOS)
		mark2 = mark + strlen(mark);

	else 
	{
		if (lastword >= 0)
			mark2 = move_to_word(start, lastword + 1);
		else
		{
			mark2 = (char *)start + strlen(start);
			move_word_rel(start, &mark2, lastword);
		}

		while (mark2 > start && my_isspace(mark2[-1]))
			mark2--;
	}

	/* 
	 * If the end is before the string, then there is nothing
	 * to extract (this is perfectly legal, btw)
         */
	if (mark2 < mark)
		booya = m_strdup(empty_string);

	else
	{
#if 0
		/* Otherwise, copy off the string we just isolated */ 
		char tmp;
		tmp = *mark2;
		*mark2 = '\0';
		booya = m_strdup(mark);
		*mark2 = tmp;
#endif
		booya = new_malloc(mark2 - mark + 1);
		strlcpy(booya, mark, mark2 - mark + 1);
	}

	return booya;
}

/*
 * extract is a simpler version of extract2, it is used when we don't
 * want special treatment of "firstword" if it is negative.  This is
 * typically used by the word/list functions, which also don't care if
 * we strip out or leave in any whitespace, we just do what is the
 * fastest.
 */
extern char	*BX_extract(char *start, int firstword, int lastword)
{
	/* 
	 * firstword and lastword must be zero.  If they are not,
	 * then they are assumed to be invalid  However, please note
	 * that taking word set (-1,3) is valid and contains the
	 * words 0, 1, 2, 3.  But word set (-1, -1) is an empty_string.
	 */
	char *mark;
	char *mark2;
	char *booya = NULL;

	/* 
	 * before we do anything, we strip off leading and trailing
	 * spaces. 
	 *
	 * ITS OK TO TAKE OUT SPACES HERE, AS THE USER SHOULDNT EXPECT
	 * THAT THE WORD FUNCTIONS WOULD RETAIN ANY SPACES. (That is
	 * to say that since the word/list functions don't pay attention
	 * to the whitespace anyhow, noone should have any problem with
	 * those ops removing bothersome whitespace when needed.)
	 */
	while (my_isspace(*start))
		start++;
	remove_trailing_spaces(start);

	if (firstword == EOS)
	{
		mark = start + strlen(start);
		mark = move_word_rel(start, &mark, -1);
	}

	/* If the firstword is positive, move to that word */
	else if (firstword >= 0)
		mark = move_to_word(start, firstword);

	/* Its negative.  Hold off right now. */
	else
		mark = start;


	/* When we find the last word, we need to move to the 
           END of the word, so that word 3 to 3, would include
	   all of word 3, so we sindex to the space after the word
 	 */
	/* EOS is a #define meaning "end of string" */
	if (lastword == EOS)
		mark2 = start + strlen(start);
	else 
	{
		if (lastword >= 0)
			mark2 = move_to_word(start, lastword + 1);
		else
			/* it's negative -- that's not valid */
			return m_strdup(empty_string);

		while (mark2 > start && my_isspace(mark2[-1]))
			mark2--;
	}

	/* OK.. now if we get to here, then lastword is positive, so
	 * we sanity check firstword.
	 */
	if (firstword < 0)
		firstword = 0;
	if (firstword > lastword)	/* this works even if fw was < 0 */
		return m_strdup(empty_string);

	/* If the end is before the string, then there is nothing
	 * to extract (this is perfectly legal, btw)
         */
#if 0
	booya = NULL;
#endif
	if (mark2 < mark)
		return m_strdup(empty_string);
	
	booya = new_malloc(mark2 - mark + 1);
	strlcpy(booya, mark, mark2 - mark + 1);
#if 0
		malloc_strcpy(&booya, empty_string);
	else
	{
		/* Otherwise, copy off the string we just isolated */ 
		char tmp;
		tmp = *mark2;
		*mark2 = '\0';
		malloc_strcpy(&booya, mark);
		*mark2 = tmp;
	}
#endif
	return booya;
}
