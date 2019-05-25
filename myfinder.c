#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <dlfcn.h>
#include <execinfo.h>
int 
pthread_mutex_lock (pthread_mutex_t *mutex)
{
	int  (*lockp)(pthread_mutex_t *mutex) ; 
	char * error ;
	
	lockp = dlsym(RTLD_NEXT, "pthread_mutex_lock") ;
	if ((error = dlerror()) != 0x0) 
		exit(1) ;

	char buf[50] ;
	snprintf(buf, 50, "in\n") ;
	fputs(buf, stderr) ;
	return 77 ; 	
}
