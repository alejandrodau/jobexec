#include <stdlib.h>
#include <sys/time.h>

void timerStart(struct timeval *time) {

   gettimeofday(time, NULL);

}


double timerGet(struct timeval *start) {
   struct timeval time;

   gettimeofday(&time, NULL);

   double r;
   if (time.tv_usec < start->tv_usec)
    {
      /* Manually carry a one from the seconds field.  */
      time.tv_usec += 1000000;
      time.tv_sec -= 1;
    }

   r = time.tv_sec - start->tv_sec;
   r += (time.tv_usec - start->tv_usec) / 1000000.0;

   return r;
}


