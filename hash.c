#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

unsigned long hashdjb(unsigned char *str)
{
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
	hash = hash * 33 + c;

    return hash;
}

char *hashstr(unsigned char *str, unsigned int len)
{

    unsigned long hash;
    char *hashchar;
    char table[] = "0123456789abcdefghjkmnopqrstuvwx";

    hashchar = malloc((len + 1) * sizeof(char));

    if (hashchar == NULL)
	return NULL;

    hash = hashdjb(str);

    hashchar[len] = '\0';

//debug:   printf("sizof table: %ld , hash: %ld\n", sizeof(table), hash);

    while (len > 0) {
	len--;
	hashchar[len] = table[hash & 31];
	hash = hash >> 5;
    }

    return hashchar;
}
