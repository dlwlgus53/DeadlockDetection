#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <dlfcn.h>
#include <execinfo.h>

struct Node{
	int count;
	pthread_mutex_t *mutex;
	struct Node *next;
}Node;

struct Node* head;
//print value in linked list
void
printer(){
	struct Node* current = head;
	while(current !=NULL){
		printf(" %d->", current->count);
		current = current->next;
	}
	printf("\n");
	return;
}


void
detect(){//detect deadlock
	printer();
	struct Node* current = head;
	while(current !=NULL){
		if(current->count >=0){
			return;
		}
		current = current->next;
	}
	char buf[50];
	snprintf(buf,50,"deadlock\n");
	fputs(buf,stderr);
	return;
}
	
void/*push item in linked list*/
push(struct Node** head_ref,pthread_mutex_t *mutex) 
{ 
    /* allocate node */
    struct Node* new_node = 
            (struct Node*) malloc(sizeof(struct Node)); 
  
    /* put in the key  */
    new_node->mutex  = mutex; 
    new_node->count = 0; 
    /* link the old list off the new node */
    new_node->next = (*head_ref); 
  
    /* move the head to point to the new node */
    (*head_ref)    = new_node; 
}

int
lock_find_mutex(pthread_mutex_t *mutex){
	struct Node* current = head;
    	int find =0;
	while (current != NULL) 
    	{	
        	if (current->mutex == mutex){
			current->count = current->count-1;
			return 1; 
            	}
        	current = current->next; 
    	}

	push(&head,mutex);
				 
	return 0;	 
}

int
ulock_find_mutex(pthread_mutex_t *mutex){
        struct Node* current = head;
        int find =0;
        while (current != NULL)
        {
                if (current->mutex == mutex){
			if(current->count <=0){
				current->count = current->count + 1;
				return 1;
			}
                        else return 1;
                }
                current = current->next;
        }

        return 0;
}
int 
pthread_mutex_lock (pthread_mutex_t *mutex)
{
	int  (*lockp)(pthread_mutex_t *mutex) ; 
	char * error ;
	
	lockp = dlsym(RTLD_NEXT, "pthread_mutex_lock") ;
	if ((error = dlerror()) != 0x0) 
		exit(1) ;
    
	/*find mutex in linked list and update status or push new node*/
	lock_find_mutex(mutex);

	/*detect deadlock*/
	detect();
		
	lockp(mutex);
	return 0; 	
}

int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
	int  (*ulockp)(pthread_mutex_t *mutex) ; 
	char * error ;
	
	ulockp = dlsym(RTLD_NEXT, "pthread_mutex_unlock") ;
	if ((error = dlerror()) != 0x0) 
		exit(1) ;
    
	ulock_find_mutex(mutex);
	char buf[50] ;
	snprintf(buf, 50, "out\n") ;
	fputs(buf, stderr) ;
	ulockp(mutex);
	return 77 ; 	
}
