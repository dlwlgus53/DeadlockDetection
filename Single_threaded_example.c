#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

pthread_mutex_t a = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t b = PTHREAD_MUTEX_INITIALIZER;


void * 
single_thread(void * ptr) 
{
	pthread_mutex_lock(&a);
    pthread_mutex_lock(&b);
    pthread_mutex_unlock(&b);
    pthread_mutex_unlock(&a);
    pthread_mutex_lock(&b);
    pthread_mutex_lock(&a);
    pthread_mutex_unlock(&a);
    pthread_mutex_unlock(&b);

	return 0x0 ;
}


int 
main() 
{
	pthread_t test ;

		pthread_create(&(test), 0x0 , single_thread, 0x0) ;
		pthread_join(test, 0x0) ;
	
	exit(0) ;
}
