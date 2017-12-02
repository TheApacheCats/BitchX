#ifndef SSL_H_
#define SSL_H_

#if defined(HAVE_LIBSSL) && !defined(IN_MODULE)

#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#ifndef TRUE
#define TRUE 0
#endif
#ifndef FALSE
#define FALSE 1
#endif

void SSL_show_errors(void);

/* Make these what you want for cert & key files */

/*extern    SSL_CTX* ctx;*/
/*extern    SSL_METHOD *meth;*/

#endif
#endif
