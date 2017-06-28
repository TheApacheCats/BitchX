/*
 * crypt.c: handles some encryption of messages stuff. 
 *
 * Written By Michael Sandrof
 *
 * Copyright(c) 1990 
 *
 * See the COPYRIGHT file, or do a HELP IRCII COPYRIGHT 
 */


#include "irc.h"
static char cvsrevision[] = "$Id$";
CVS_REVISION(encrypt_c)
#include "struct.h"

#include "encrypt.h"
#include "vars.h"
#include "ircaux.h"
#include "list.h"
#include "output.h"
#include "newio.h"
#define MAIN_SOURCE
#include "modval.h"

static	void	add_to_crypt (char *, char *);
static	int	remove_crypt (char *);

/*
 * Crypt: the crypt list structure,  consists of the nickname, and the
 * encryption key 
 */
typedef struct	CryptStru
{
	struct	CryptStru *next;
	char	*nick;
	char	*key;
}	Crypt;

/* crypt_list: the list of nicknames and encryption keys */
static	Crypt	*crypt_list = NULL;

/*
 * add_to_crypt: adds the nickname and key pair to the crypt_list.  If the
 * nickname is already in the list, then the key is changed the the supplied
 * key. 
 */
static	void add_to_crypt(char *nick, char *key)
{
	Crypt	*new;

	if ((new = (Crypt *) remove_from_list((List **) &crypt_list, nick)) != NULL)
	{
		new_free(&(new->nick));
		new_free(&(new->key));
		new_free((char **)&new);
	}
	new = (Crypt *) new_malloc(sizeof(Crypt));
	malloc_strcpy(&(new->nick), nick);
	malloc_strcpy(&(new->key), key);
	add_to_list((List **) &crypt_list, (List *) new);
}

/*
 * remove_crypt: removes the given nickname from the crypt_list, returning 0
 * if successful, and 1 if not (because the nickname wasn't in the list) 
 */
static	int remove_crypt(char *nick)
{
	Crypt	*tmp;

	if ((tmp = (Crypt *) list_lookup((List **) &crypt_list, nick, !USE_WILDCARDS, REMOVE_FROM_LIST)) != NULL)
	{
		new_free(&(tmp->nick));
		new_free(&(tmp->key));
		new_free((char **)&tmp);
		return (0);
	}
	return (1);
}

/*
 * is_crypted: looks up nick in the crypt_list and returns the encryption key
 * if found in the list.  If not found in the crypt_list, null is returned. 
 */
const char *is_crypted(char *nick)
{
	Crypt	*tmp;

	if (!crypt_list)
		return NULL;
	if ((tmp = (Crypt *) list_lookup((List **) &crypt_list, nick, !USE_WILDCARDS, !REMOVE_FROM_LIST)) != NULL)
		return (tmp->key);
	return NULL;
}

/*
 * encrypt_cmd: the ENCRYPT command.  Adds the given nickname and key to the
 * encrypt list, or removes it, or list the list, you know. 
 */
BUILT_IN_COMMAND(encrypt_cmd)
{
	char	*nick,
	*key;

	if ((nick = next_arg(args, &args)) != NULL)
	{
		if ((key = next_arg(args, &args)) != NULL)
		{
			add_to_crypt(nick, key);
			say("%s added to the crypt with key %s", nick, key);
		}
		else
		{
			if (remove_crypt(nick))
				say("No such nickname in the crypt: %s", nick);
			else
				say("%s removed from the crypt", nick);
		}
	}
	else
	{
		if (crypt_list)
		{
			Crypt	*tmp;

			say("The crypt:");
			for (tmp = crypt_list; tmp; tmp = tmp->next)
				put_it("%s with key %s", tmp->nick, tmp->key);
		}
		else
			say("The crypt is empty");
	}
}

extern	void BX_my_encrypt (char *str, int len, const char *key)
{
	int	key_len,
		key_pos,
		i;
	char	mix,
		tmp;

	if (!key)
		return;
		
	key_len = strlen(key);
	key_pos = 0;
	mix = 0;
	for (i = 0; i < len; i++)
	{
		tmp = str[i];
		str[i] = mix ^ tmp ^ key[key_pos];
		mix ^= tmp;
		key_pos = (key_pos + 1) % key_len;
	}
	str[i] = (char) 0;
}

extern	void BX_my_decrypt(char *str, int len, const char *key)
{
	int	key_len,
		key_pos,
		i;
	char	mix,
		tmp;

	if (!key)
		return;
		
	key_len = strlen(key);
	key_pos = 0;
	/*    mix = key[key_len-1]; */
	mix = 0;
	for (i = 0; i < len; i++)
	{
		tmp = mix ^ str[i] ^ key[key_pos];
		str[i] = tmp;
		mix ^= tmp;
		key_pos = (key_pos + 1) % key_len;
	}
	str[i] = (char) 0;
}

