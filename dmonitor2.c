
/* new predictor*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
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

/* sturct for mutex connecing information */
struct Edge
{
	long time;
	pthread_t thid;
	pthread_mutex_t *src;
	pthread_mutex_t *dest;
	pthread_mutex_t *guard;
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

 

/* signal to detect debugging */
void sighandler(int sig)
{
    void *array[10];
    size_t size;
    char **strings;
    size_t i;

 

    size = backtrace(array, 10);

    strings = backtrace_symbols(array, size);

 


    for(i = 2; i < size; i++)

        printf("%lu: %s\n", i - 2, strings[i]);

 

    free(strings);

 

    exit(1);
}

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

/* This is for make 2d Array */
void **create2darray(int count) /* Allocate the array */
{
    /* Check if allocation succeeded. (check for NULL pointer) */
    int i;
    adjArray = malloc(count*sizeof(int *));
    for(i = 0 ; i < count ; i++)
        adjArray[i] = malloc( count*sizeof(int) );
}


/* mutexArray() makes index-mutex array. ex(1-A, 2-C, 3-B)) */
int mutexArray(){
    
    int lastPoint=0;
    for(int i=0 ;i<threadNum; i++){
        if(monitor[i][0].mutex == NULL){
            return lastPoint;
        }
	for(int j=0; j<mutexNum; j++){
            if(monitor[i][j].mutex == NULL)    break;   
	 else{
                int diff =1;
                for(int k=0; k<lastPoint; k++){
                    if( monitor[i][j].mutex == mArr[k])
                        diff=0;
                }
                if(diff ==1){
                    mArr[lastPoint] = monitor[i][j].mutex;
                    lastPoint++;
                }
            }
        }
    }
	return lastPoint;
	
}
int getNum(pthread_mutex_t *mutex, int length){
	for(int i=0; i<length; i++){
        if(mArr[i] == mutex){
            return i;
	}
    }
}

void MakeAdjArray(){
    int count = mutexArray();
    create2darray(count);
    
    for(int i=0; i<count; i++){
        for(int j=0; j<count; j++){
            adjArray[i][j] =0;
        }
    }
    
    for(int i=0 ;i<threadNum; i++){
        if(monitor[i][0].thid == 0){
            return;
        }
    for(int j=0; j<mutexNum; j++){
        if(monitor[i][j+1].mutex == NULL)    break;
        if(monitor[i][j].mutex == monitor[i][j+1].mutex)	break;
	else{   
                int src = getNum(monitor[i][j].mutex, count);
                int dest = getNum(monitor[i][j+1].mutex, count);
                adjArray[src][dest] = 1;
            }
        }
    }
    
   
    return;
}
//find cycle
int cycleFinder(int* checker,int i, int count){
  //checker[i] = index;
int j=0;
    for(j=0; j<count; j++){
        if(adjArray[i][j] == 1){
            if(checker[j] == 1){
                /*printf("|");
                for (int j=0; j<count; j++){
                    printf("%d ", checker[j]);
                }
                printf("|");*/
                return j;
            }
            else{
		checker[i] = j;
                return cycleFinder(checker, j,count);
            }
            
        }
    }
    if(j==count){
	return -1;
    }
}

int
countMutex(int arr[], int index, int count){
	int check[count];
	for(int i=0; i<count; i++)	check[i] =0;
	int mutexNode=0;
	while(1){
		check[index]=1;
		mutexNode++;
		index = arr[index];
		if(check[index] != 0)	break;
	}
	return mutexNode;
}
void
fillThidnGuardnTime(pthread_mutex_t *src, pthread_mutex_t *dest,struct Edge* edge){
	for(int i=0 ;i<threadNum; i++){
        if(monitor[i][0].thid == 0){
		printf("find error..\n");
            	return;
        }
        for(int j=0; j<mutexNum; j++){
            if(monitor[i][j+1].thid == 0)    break;
            else{
			if(monitor[i][j].mutex == src && monitor[i][j+1].mutex == dest){
				printf("(%d , %d)\n", i, j);
				edge->thid = monitor[i][j].thid;
				edge->time = monitor[i][j].time;
				if(j>0)	edge->guard = monitor[i][j-1].mutex;
				return;
			} 
		}
	    }
        }
	return;
}

void
fillEdges(int* checker, int index, struct Edge* edges,int cycledMutex){
	for(int i=0; i<cycledMutex; i++){
		int srcNum = checker[index];
		int destNum = checker[srcNum];

       		pthread_mutex_t *src = mArr[srcNum];
        	pthread_mutex_t *dest = mArr[destNum];

		//find a-b in array
		fillThidnGuardnTime(src, dest,&edges[i]);
		//add to edges
		edges[i].src = src;
		edges[i].dest = dest;
		

		index = checker[destNum];
	}
}
int check1(struct Edge edges[], int count){
	int diff =1;
	for(int i=0; i<count-1; i++){
		for(int j=i+1; j<count; j++){
			if(edges[i].thid == edges[j].thid)	diff=0;
			}
		}
	return diff;
}


/*checker for find guard lock*/
int check2(struct Edge edges[],int count){
	/* 1 :danger(not same guard) 0 : safe(same guard)*/
	for(int i=0; i<count; i++){
		if(edges[i].guard == NULL){
			return 1;
		}
		for(int j=i+1; j<count; j++){
			printf("in ch2\n");
			printf("(%d %d)\n", i, j);
			if(edges[j].guard == NULL)	return 1;
			if(edges[i].guard != edges[j].guard) return 1;
		}
	}
	return 0;
			
}

int check3(struct Edge edges[],int count){
	int danger=1;
	for(int i=0; i<threadNum; i++){
		if(thEdges[i].src == 0) break;
		for(int j=0; j<count; j++){
			if(thEdges[i].src == edges[j].thid){
			if(thEdges[i].time > edges[j].time){
				danger=0; //
			}
			}
		}
	}
			
	

	return danger;
		
}
void printer(){
	printf("=======================\n");
    for(int i=0 ;i<threadNum; i++){
        printf("\n");
	if(monitor[i][0].thid == 0)    return;
        printf("[%d] ", i);
	for(int j=0; j<mutexNum; j++){
            if(monitor[i][j].thid == 0)    break;
            else{
                printf("%lu ", monitor[i][j].thid);
            }
        }
    }
}


/* mutex for mutex_lock hooking function */

int main(pthread_mutex_t *mutex)
{
	/*start read file*/
	FILE *fp = fopen("dmonitor.trace", "r");
	int i=0;
	int j=0;

	/* read threadID data */
	for(i=0; i<threadNum; i++){
		for(j=0; j<mutexNum; j++){
			fscanf( fp,"%lu", &monitor[i][j].thid );
		//	printf("%lu ", monitor[i][j].thid);
		}
		//printf("\n");

	}

	/* read time data */
	for(i=0; i<threadNum; i++){
		for(j=0; j<mutexNum; j++){
			fscanf( fp,"%lu", &monitor[i][j].time );
			//printf("%lu ", monitor[i][j].time);
		}
		//printf("\n");
	}

	/* read mutex data */
	int temp=0;
	for(i=0; i<threadNum; i++){
		for(j=0; j<mutexNum; j++){
			if(monitor[i][j].thid ==0 ){	fscanf(fp,"%d", &temp);} //printf("0 ");}
			else{	fscanf(fp,"%p", &monitor[i][j].mutex);
			//printf("%p ",monitor[i][j].mutex);
			}
		}
		//printf("\n");
	}

	/* read src thid data */

	for(i=0; i<threadNum; i++){
		fscanf(fp, "%lu", &thEdges[i].src);
		//printf("%lu ", thEdges[i].src);
	}

	/* read dest thid data */
	for(i=0; i<threadNum; i++){
		fscanf(fp, "%lu", &thEdges[i].dest);
		//printf("%lu ", thEdges[i].dest);
	}

	/* read time data */
	for(i=0; i<threadNum; i++){
		fscanf(fp, "%lu", &thEdges[i].time);
		//printf("%lu ", thEdges[i].time);
	}	
		
		/* monitor -> adjArray */	
        	MakeAdjArray();
		/*  make dictionary and get count */
        	int count =mutexArray();
       		
		/* make checker for cycleFinder() */
		int checker[count];
		for(int i=0; i<count; i++){
        	    checker[i] = 0;
	        }

		/* find cycle */
		int index;
		for(int i=0; i<count; i++){
			index=cycleFinder(checker,0,count);
			if(index!=-1) break;		
		}

		//printer();
		if(index!=-1){
			int cycledMutex = countMutex(checker,index,count);
			printf("%d", cycledMutex);
			struct Edge edges[cycledMutex];
			fillEdges(checker,index,edges,cycledMutex);
			if(check1(edges, cycledMutex)&&check2(edges, cycledMutex)&&check3(edges, cycledMutex)){
				
				printf("checker %d %d %d\n", check1(edges,cycledMutex), check2(edges, cycledMutex), check3(edges,cycledMutex));
				printf("danger\n");
				int i ;
                		void * arr[10] ;
                		char ** stack ;


                		size_t sz = backtrace(arr, 10) ;
                		stack = backtrace_symbols(arr, sz) ;
                			
				FILE * fp;
   				fp = fopen ("dmonitor.trace","w");
 				fprintf(fp,"%d\n", count);
   				for(i = 0; i < sz;i++){		
       					fprintf (fp, "%s\n", stack[i]);
   				}
 
   				fclose (fp);
			}
		}

	return 0;
}


