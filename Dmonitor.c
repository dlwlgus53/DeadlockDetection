/* monitor */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <sys/timeb.h>  /* ftime, timeb (for timestamp in millisecond) */
#include <sys/time.h>
#define _POSIX_C_SOURCE 200809L
#include <inttypes.h>
#include <math.h>

#define mutexNum 100
#define threadNum 10

struct timespec spec;
/* node for monitor array */
struct Mnode
{       
        long time;
	pthread_t thid; // initialized 0 
        pthread_mutex_t *mutex;
};

struct thEdge
{
 	long time;
	pthread_t src; 
	pthread_t dest;
};


/* monitor array */
struct Mnode monitor[threadNum][mutexNum];
/* arr for mutex, by this arr, mutex get id number */
pthread_mutex_t*mArr[mutexNum];

/* array for make adjArray */
int ** adjArray;

/* edges of thread, I didnt't use it but it works */
struct thEdge thEdges[threadNum];

 

/* Add mutex to monitor array. Same row has same thread_id, and new mutex goes next column index */
void addToMonitor( pthread_t thid,pthread_mutex_t *mutex){//add to monitor array
    for(int i=0; i<threadNum; i++){
        if(monitor[i][0].thid == thid){
            for(int j=0; j<mutexNum; j++){
                if(monitor[i][j].mutex == NULL){//������ ĭ �߰�
                    monitor[i][j].thid = thid;
                    monitor[i][j].mutex = mutex;
                    clock_gettime(CLOCK_REALTIME, &spec);
 			monitor[i][j].time = (spec.tv_nsec);
			return;
                }        
            }
        }else if(monitor[i][0].mutex == NULL){
            monitor[i][0].thid = thid;
            monitor[i][0].mutex = mutex;
		clock_gettime(CLOCK_REALTIME, &spec);
                monitor[i][0].time = (spec.tv_nsec);           return;
        }
    }
}


void printer()
{
	FILE *fp = fopen("dmonitor.trace","w");
		/*start file wirte*/
		/* monitor.thid */
		for(int i=0; i<threadNum;i++){
			for(int j=0; j<mutexNum; j++){
				fprintf(fp, "%lu ", monitor[i][j].thid);
			}
		}
		/* monitor.time */
		fprintf(fp,"\n");
		for(int i=0; i<threadNum; i++){
			for(int j=0; j<mutexNum; j++){
				fprintf(fp, "%lu ", monitor[i][j].time);
			}
		}
		/* monitor.mutex */
		fprintf(fp, "\n");
		for(int i=0; i<threadNum; i++){
			for(int j=0; j<mutexNum; j++){
				if(monitor[i][j].mutex == NULL)	fprintf(fp, "0 ");
				else fprintf(fp, "%p ", monitor[i][j].mutex);
			}
		}
		/* thread.src */
		fprintf(fp, "\n");
		for(int i=0; i<threadNum; i++){
			fprintf(fp, "%lu ", thEdges[i].src);
		}
		/* thread.dest */
		fprintf(fp, "\n");
		for(int i=0; i<threadNum; i++){
			fprintf(fp, "%lu ", thEdges[i].dest);
		}

		/* thread.time*/
		fprintf(fp, "\n");
		for(int i=0; i<threadNum; i++){
			fprintf(fp, "%lu ", thEdges[i].time);
		}
		
		fclose(fp);
	

}

/* mutex for mutex_lock hooking function */
pthread_mutex_t localmutex = PTHREAD_MUTEX_INITIALIZER;
int
pthread_mutex_lock (pthread_mutex_t *mutex)
{

	printf("?");
	static __thread int n_mutex = 0 ; //https://gcc.gnu.org/onlinedocs/gcc-3.3/gcc/Thread-Local.html
	n_mutex += 1 ;

	int  (*lockp)(pthread_mutex_t *mutex) ;
        char * error ;

       	lockp = dlsym(RTLD_NEXT, "pthread_mutex_lock") ;
        if ((error = dlerror()) != 0x0)
                exit(1); 

	if (n_mutex == 1) {
		pthread_mutex_lock(&localmutex);			
		//add to monitor array
		addToMonitor(pthread_self(), mutex);
		printer();
		pthread_mutex_unlock(&localmutex);
	}		
	

	n_mutex-= 1 ;
	return 0;
}
/*insert pthread_id to thArr*/

void addTothEdges( pthread_t present_th, pthread_t new_th){//add to monitor array
    for(int i=0; i<threadNum; i++){
		if(thEdges[i].src == 0){
			thEdges[i].src = present_th;
			thEdges[i].dest = new_th;
			clock_gettime(CLOCK_REALTIME, &spec);
			thEdges[i].time =(spec.tv_nsec) ;
			
			return;
		}
	}
}
	
/* pthread array name : thEgeds */

int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start)(void *), void *arg){
	printf("create\n");
	static int (*pthread_createp)(pthread_t *thread, const pthread_attr_t *attr, void *(*start)(void *), void *arg) ;
	char * error ;
	
	pthread_createp = dlsym(RTLD_NEXT, "pthread_create") ;
	if ((error = dlerror()) != 0x0) 
		exit(1) ;
		
	int return_value = pthread_createp(thread, attr, start, arg);
	
	pthread_t src = pthread_self();

	addTothEdges(src, *thread);
 	printer();		
	return return_value ; 
}
