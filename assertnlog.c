#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "jobexec.h"

void assertnlog(int b, char *msg) {
   if (!b) {
        fprintf(stderr, "%s\n", msg);

        if (errno != 0) {
                perror(NULL);
        }
        exit(ERRJOBEXEC);
   }
}

void assertnlog2(int b, char *msg1, char *msg2) {
   if (!b) {
        fprintf(stderr, "%s: %s\n", msg1, msg2);

        if (errno != 0) {
                perror(NULL);
        }
        exit(ERRJOBEXEC);
   }
}

