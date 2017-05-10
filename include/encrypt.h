/*
 * encrypt.h: header for encrypt.c 
 *
 * Written By Michael Sandrof
 *
 * Copyright(c) 1990 
 *
 * See the COPYRIGHT file, or do a HELP IRCII COPYRIGHT 
 *
 * @(#)$Id$
 */
#ifndef ENCRYPT_H_
#define ENCRYPT_H_

	char	*crypt_msg (char *, char *);
	char	*decrypt_msg (char *, char *);
	void	encrypt_cmd (char *, char *, char *, char *);
	char	*is_crypted (char *);
	void	BX_my_decrypt (char *, int, char *);
	void	BX_my_encrypt (char *, int, char *);

#define CRYPT_HEADER ""
#define CRYPT_HEADER_LEN 5

#endif /* ENCRYPT_H_ */
