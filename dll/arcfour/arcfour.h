typedef unsigned char arcword;		/* 8-bit groups */

typedef struct {
   arcword state[256], i, j;
} arckey;

/* Prototypes */
static inline void arcfourInit(arckey *, void *, unsigned short);
static inline char *arcfourCrypt(arckey *, char *, int);
static int send_dcc_encrypt (int, int, char *, int);
static int dcc_schat_input (int, int, char *, int, int);
static int start_dcc_crypt (int, int, unsigned long, unsigned short);
static int end_dcc_crypt (int, unsigned long, unsigned short);
void dcc_sdcc (char *, char *);
