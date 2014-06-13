// in hash.c:
char *hashstr(unsigned char *str, unsigned int len);

// in assertnlog.c:
#define ERRJOBEXEC 211
void assertnlog(int b, char *msg);
void assertnlog2(int b, char *msg1, char *msg2);
