#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

pthread_mutex_t x;
pthread_mutex_t y;
pthread_mutex_t z;

pthread_mutex_init(&x);
pthread_mutex_init(&y);
pthread_mutex_init(&z);

void * 
threading1(void * ptr) 
{
	pthread_mutex_lock(&x);
    pthread_mutex_lock(&y);
    pthread_mutex_lock(&z);
    pthread_mutex_unlock(&z);
    pthread_mutex_unlock(&y);
    pthread_mutex_unlock(&x);
   

	return 0x0 ;
}

void * 
threading2(void * ptr) 
{
	pthread_mutex_lock(&x);
    pthread_mutex_lock(&z);
    pthread_mutex_lock(&y);
    pthread_mutex_unlock(&y);
    pthread_mutex_unlock(&z);
    pthread_mutex_unlock(&x);

	return 0x0 ;
}


int 
main() 
{
	pthread_t thread1 ;
    pthread_t thread2 ;

		pthread_create(&(thread1), 0x0 ,threading1 , 0x0) ;
        pthread_create(&(thread2), 0x0 , threading2, 0x0) ;
		pthread_join(&thread1, 0x0) ;
        pthread_join(&thread2, 0x0) ;
	
	exit(0) ;
}