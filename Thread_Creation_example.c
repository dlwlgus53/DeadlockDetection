#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

pthread_mutex_t x = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t y = PTHREAD_MUTEX_INITIALIZER;;



void *
threading2(void * ptr) 
{
	pthread_mutex_lock(&y);
    pthread_mutex_lock(&x);
    pthread_mutex_unlock(&x);
    pthread_mutex_unlock(&y);

	return 0x0 ;
}

void *
threading1(void * ptr)
{
    pthread_t thread2 ;
        pthread_mutex_lock(&x);
    pthread_mutex_lock(&y);
    pthread_mutex_unlock(&y);
    pthread_mutex_unlock(&x);

        pthread_create(&(thread2), 0x0 ,threading2 , 0x0) ;
        pthread_join(thread2, 0x0) ;

        return 0x0 ;
}
int 
main() 
{
	pthread_t thread1 ;
    

		pthread_create(&(thread1), 0x0 ,threading1 , 0x0) ;
		pthread_join(thread1, 0x0) ;
        
	
	exit(0) ;
}
