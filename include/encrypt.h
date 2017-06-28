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

void encrypt_cmd(char *, char *, char *, char *);
const char *is_crypted(char *);
void BX_my_decrypt(char *, int, const char *);
void BX_my_encrypt(char *, int, const char *);

#define CRYPT_HEADER ""
#define CRYPT_HEADER_LEN 5

#endif /* ENCRYPT_H_ */
