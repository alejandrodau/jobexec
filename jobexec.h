// config constants:
#define SYSLOG_ON 1
#define SHELL "/bin/sh"
#define BASEDIR "/var/tmp/jobexec"
#define MAXDIRLEN 512
#define MAXCMDLEN 512


//-----------------------

#ifdef SYSLOG_ON
#include <syslog.h>
#endif

#include <sys/wait.h>

// in hash.c:
char *hashstr(unsigned char *str, unsigned int len);

// in assertnlog.c:
#define ERRJOBEXEC 211
void assertnlog(int b, char *msg);
void assertnlog2(int b, char *msg1, char *msg2);
