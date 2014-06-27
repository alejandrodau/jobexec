#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "timer.h"
#include "jobexec.h"


extern char **environ;
char dir[MAXDIRLEN];


unsigned int getParamsLen(char **params) {
   unsigned int c;
   int j, i;
   
   c=0;
   j=0; 
   while (params[j] != NULL) {
     c += 2; // quotation marks
     c++; // space
     i=0;
     while (params[j][i] != '\0') {
        c++;
        i++;
     }
     j++;
   }
   return c; 
}

char **getShellParams(char **params) {
   static char *newparams[3];
   char *cmd;

   newparams[0]=SHELL;
   newparams[1]="-c";

   int maxlen=getParamsLen(params);
   maxlen += 1;

   cmd=malloc(maxlen*sizeof(char)); 
   assertnlog2(cmd != NULL, "Failed to malloc for shell execution of", params[1]);

   int i,j,l;

   i=0; j=0; 
   while (params[j] != NULL && i < maxlen) {
      l=0;
      // if its a parameter to the command, use quotation marks
      if (j != 0 && i < maxlen)  cmd[i++] = '"'; 
      while (params[j][l] != '\0' && i < maxlen) {
         if (params[j][l] == '"') cmd[i++] = '\\';
         if ( i < maxlen ) cmd[i++] = params[j][l];
         l++;
      }
      // close quotation marks
      if (j != 0 && i < maxlen)  cmd[i++] = '"';
      if (i < maxlen) cmd[i++] = ' '; 

      j++;
   }
   assertnlog ( i < maxlen, "cmd maxlen calculation error!");

   cmd[i]='\0';
   newparams[2]=cmd;

   return newparams;
}

void createOutputDir(int argc, char* argv[]){

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

           // the next if is to skip the path of the command in the dirname
           if (l==1 && c=='/')  j = 0; 

	   i++;
	}
  }	

  cmdline[j]=0;
  hash = hashstr(cmdline, 5);
  assertnlog(hash != NULL, "malloc hashstr");

  // create jobexec dir
  snprintf(dir, sizeof(dir), "%s", BASEDIR);
  if( access( dir, F_OK ) == -1 ) {
    assertnlog2(!mkdir(dir, S_IRWXU), "mkdir BASEDIR", dir);
    assertnlog2(!chmod(dir, S_IRWXU | S_IRWXG | S_IRWXO| S_ISVTX), "chmod BASEDIR", dir);
  } 

  l = strlen(dir);
  //shorten the cmdline string to be used in dir
  cmdline[16]='\0';
  snprintf(&dir[l], sizeof(dir)-l, "/%ju.%s-%s", time(NULL), cmdline, hash );
  if( access( dir, F_OK ) == -1 ) {
    // create dir
//    printf("making dir: %s\n", dir);
    assertnlog2(!mkdir(dir, 00755), "mkdir dir", dir);
  } 

} 


int main(int argc, char *argv[]) {

   int fd1, fd2, l;
   FILE *f;
   char tmpstr[sizeof(dir)+20];
 
// usage:
   if (argc < 2) { 
	printf("usage: %s <command> <cmd param 1> <cmd param 2> ...\n", argv[0]);
	printf("\nmore information: man %s\n", argv[0]);
	exit(0);
   }

   createOutputDir(argc, argv);

// write command file
   snprintf(tmpstr, sizeof(tmpstr), "%s/command", dir);
   f = fopen(tmpstr, "w");
   assertnlog2(f != NULL, "fopen outfile", tmpstr);

   for (l=1; l<argc; l++) {
	fprintf( f, "%s ", argv[l]);
   }
   fprintf(f, "\n");
   fclose(f);


// create stdout file
   snprintf(tmpstr, sizeof(tmpstr), "%s/stdout", dir);
   fd1 = open(tmpstr, O_WRONLY | O_CREAT, 00644);
   assertnlog2(fd1 >= 0, "fopen outfile", tmpstr);

// create stderr file
   snprintf(tmpstr, sizeof(tmpstr), "%s/stderr", dir);
   fd2 = open(tmpstr, O_WRONLY | O_CREAT, 00644);
   assertnlog2(fd2 >= 0, "fopen outfile", tmpstr);
   

   pid_t childPID;
   childPID = fork();

   assertnlog(childPID >= 0, "fork");

   if(childPID == 0) // child process
        {
	    assertnlog(dup2(fd1, STDOUT_FILENO) != -1, "error duping stdout");
	    assertnlog(dup2(fd2, STDERR_FILENO) != -1, "error duping stderr");

	    close(fd1);
	    close(fd2);
	    char *cmd;
	    char **params;

            cmd=argv[1];
            params=&argv[1];

	    // if there is no path specified for command, invoke trhough shell
	    if (cmd[0] != '/') {
		cmd=SHELL;
		params=getShellParams(params);
	    }

            execve(cmd, params, environ);
            assertnlog2(0, "Exec failed, command not found", argv[1]);
	    // shouldnt be reachable, but just in case:
	    exit(ERRJOBEXEC);
        }
        else //Parent process
        {
	    // create 'running' file
	    snprintf(tmpstr, sizeof(tmpstr), "%s/running", dir);
    	    f = fopen(tmpstr, "w");
	    assertnlog2(f != NULL, "fopen failed", tmpstr);
	    fprintf(f, "%d\n", childPID);
	    fclose(f);

	    struct timeval timer;
	    timerStart(&timer);
	    
#ifdef SYSLOG_ON
	    syslog(LOG_INFO, "starting %s... Output: %s", argv[1], dir);
#endif

	    int status;
	    wait(&status);

	    double exectime = timerGet(&timer);
	    // delete 'running' file
	    unlink(tmpstr);

            // save 'exec_time'
	    snprintf(tmpstr, sizeof(tmpstr), "%s/exec_time", dir);
	    f = fopen(tmpstr, "w");
	    assertnlog2(f != NULL, "fopen failed", tmpstr);
	    fprintf(f, "%.6f\n", exectime);
	    fclose(f);


	    // if exit status is ERRJOBEXEC, command failed to execute
	    if (WEXITSTATUS(status) == ERRJOBEXEC) {

		// create 'failed' file
		snprintf(tmpstr, sizeof(tmpstr), "%s/failed", dir);
		f = fopen(tmpstr, "w");
		assertnlog2(f != NULL, "fopen failed", tmpstr);
		fclose(f);

#ifdef SYSLOG_ON
	    	   syslog(LOG_ERR, "%s failed to execute. Output: %s", argv[1], dir);
#endif

		exit (-1);
	    }
//	    printf("\n cmd %s pid %d exited %d with status %d (%d)\n", argv[1], childPID, WIFEXITED(status), WEXITSTATUS(status), status);
	   // write finished file
	   snprintf(tmpstr, sizeof(tmpstr), "%s/finished", dir);
	   f = fopen(tmpstr, "w");
	   assertnlog2(f != NULL, "fopen finished", tmpstr);
	   fprintf(f, "%d\n", WEXITSTATUS(status));
	   fclose(f);

	   // write bad_exit_status if status != 0
	   if (WEXITSTATUS(status) != 0) {
	           snprintf(tmpstr, sizeof(tmpstr), "%s/bad_exit_status", dir);
		   f = fopen(tmpstr, "w");
		   assertnlog2(f != NULL, "fopen ", tmpstr);
		   fprintf(f, "%d\n", WEXITSTATUS(status));
		   fclose(f);
	   }

        }

   return 0; 
}

