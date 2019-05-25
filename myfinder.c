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
    	lockp(mutex);

	char buf[50] ;
	snprintf(buf, 50, "in\n") ;
	fputs(buf, stderr) ;
	return 77 ; 	
}

int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
	int  (*ulockp)(pthread_mutex_t *mutex) ; 
	char * error ;
	
	ulockp = dlsym(RTLD_NEXT, "pthread_mutex_unlock") ;
	if ((error = dlerror()) != 0x0) 
		exit(1) ;
    
    	ulockp(mutex);
    
	char buf[50] ;
	snprintf(buf, 50, "out\n") ;
	fputs(buf, stderr) ;
	return 77 ; 	
}
