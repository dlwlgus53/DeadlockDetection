#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

pthread_mutex_t a;
pthread_mutex_t b;

pthread_mutex_init(&a);
pthread_mutex_init(&b);

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
		pthread_join(&test, 0x0) ;
	
	exit(0) ;
}