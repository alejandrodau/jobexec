all: jobexec

clean: 
	rm jobexec *.o

jobexec: jobexec.c assertnlog.c hash.c jobexec.h
	gcc -o jobexec -O3 timer.c assertnlog.c hash.c jobexec.c
   
install: jobexec
	install -g 0 -o 0 -m 0755 jobexec /usr/local/bin
