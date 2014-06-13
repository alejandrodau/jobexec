#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "jobexec.h"

#define SHELL "/bin/sh"
#define BASEDIR "/var/tmp/jobexec"
#define MAXDIRLEN 512
#define MAXCMDLEN 512

extern char **environ;
char dir[MAXDIRLEN];


int createOutputDir(int argc, char* argv[]){

   int l,i,j;
  char cmdline[MAXCMDLEN];
  char c;
  char *hash;

  j=0;
  for (l=1; l<argc; l++) {
	i=0;
	while(argv[l][i] != '\0' && j < MAXCMDLEN) {
	   c = argv[l][i];
	   if (c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z') {
		   cmdline[j]=c;
		   j++;
	   }
	   i++;
	}
  }	

  cmdline[j]=0;
  hash = hashstr(cmdline, 5);
  assertnlog(hash != NULL, "malloc hashstr");
//  printf("%s.%s.%ju\n", cmdline, hash, time(NULL));


  snprintf(dir, sizeof(dir), "%s", BASEDIR);
  if( access( dir, F_OK ) == -1 ) {
    // create dir
//    printf("making dir: %s\n", dir);
    assertnlog2(!mkdir(dir, S_IRWXU), "mkdir BASEDIR", dir);
    assertnlog2(!chmod(dir, S_IRWXU | S_IRWXG | S_IRWXO| S_ISVTX), "chmod BASEDIR", dir);
  } 

  l = strlen(dir);
  snprintf(&dir[l], sizeof(dir)-l, "/%s.%s.%ju", cmdline, hash, time(NULL) );
  if( access( dir, F_OK ) == -1 ) {
    // create dir
//    printf("making dir: %s\n", dir);
    assertnlog2(!mkdir(dir, 00755), "mkdir BASEDIR", dir);
  } 

 
} 

int main(int argc, char *argv[]) {

   int fd;
   char tmpstr[sizeof(dir)+20];
 
   assertnlog(argc > 1, "arguments: command");

   createOutputDir(argc, argv);

   
   snprintf(tmpstr, sizeof(tmpstr), "%s/outfile", dir);
   fd = open(tmpstr, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
   assertnlog2(fd >= 0, "fopen outfile", tmpstr);
   
   
   pid_t childPID;
   childPID = fork();

   assertnlog(childPID >= 0, "fork");

   if(childPID == 0) // child process
        {
	    assertnlog(dup2(fd, STDOUT_FILENO) != -1, "error duping");

	    close(fd);

            execve(argv[1], &argv[1], environ);
            assertnlog2(0, "Exec failed, command not found", argv[1]);
        }
        else //Parent process
        {
	    int status;
	    wait(&status);
	    if (WEXITSTATUS(status) == ERRJOBEXEC) {
		/* failed to execute */
		exit (-1);
	    }
//	    printf("\n cmd %s pid %d exited %d with status %d (%d)\n", argv[1], childPID, WIFEXITED(status), WEXITSTATUS(status), status);
        }

   return 0; 
}

