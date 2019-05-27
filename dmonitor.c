#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <pthread.h>
#include <unistd.h>

#define mutexNum 100
#define threadNum 10

struct Mnode//node for monitor
{       
        pthread_t thid;
        pthread_mutex_t *mutex;
};

struct Mnode monitor[threadNum][mutexNum];
pthread_mutex_t*mArr[mutexNum];
int ** adjArray;

void addToMonitor( pthread_t thid,pthread_mutex_t *mutex){//add to monitor array
    for(int i=0; i<threadNum; i++){
        if(monitor[i][0].thid == thid){
            for(int j=0; j<mutexNum; j++){
                //thid´Â °°À»¶§ ¸¶Áö¸· Ä­À» Ã£¾Æ¼­ ³Ö¾îÁÖ¸é µÈ´Ù.
                if(monitor[i][j].thid == 0){//¸¶Áö¸· Ä­ ¹ß°ß
                    monitor[i][j].thid = thid;
                    monitor[i][j].mutex = mutex;
                    return;
                }        
            }
        }else if(monitor[i][0].thid == 0){
            //°°Àº thid°¡ ¾øÀ»¶§ -> »õ·Ó°Ô ½ÃÀÛ
            monitor[i][0].thid = thid;
            monitor[i][0].mutex = mutex;
            return;
        }
    }
}

// We return the pointer
void **create2darray(int count) /* Allocate the array */
{
    /* Check if allocation succeeded. (check for NULL pointer) */
    int i;
    adjArray = malloc(count*sizeof(int *));
    for(i = 0 ; i < count ; i++)
        adjArray[i] = malloc( count*sizeof(int) );
}


//now have to find graph adjArray
int mutexArray(){
    
    //mutex array ¾È¿¡ ¼­·Î ´Ù¸¥ mutex¸¦ ³Ö¾îÁà¾ßÇÔ.
    int lastPoint=0;
    //¹è¿­À» ¹Ýº¹¹®À» µ¹¸®´Âµ¥ tid=0À» ¸¸³ªÁö¾ÊÀ» ¶§ ±îÁö
    for(int i=0 ;i<threadNum; i++){
        if(monitor[i][0].thid == 0){
            return lastPoint;
        }
	for(int j=0; j<mutexNum; j++){
            if(monitor[i][j].thid == 0)    break;
            else{
                int diff =1;// »õ·Î¿î °ÍÀÎÁö ¾Æ´Ñ °ÍÀÎÁö¸¦ ¾Ë·ÁÁÖ´Â º¯¼ö-> »õ·Ó´Ù°í ÃÊ±âÈ­
                for(int k=0; k<lastPoint; k++){//Áö±ÝÀÇ mutex°¡ ±âÁ¸¿¡ ÀÖ´Â °ÍÀÎÁö °Ë»ç.
                    if(mArr[j]!=NULL && monitor[i][j].mutex == mArr[k])//»õ·ÓÁö ¾Ê´Ù-> diff=0 À¸·Î º¯°æ
                        diff=0;
                }
                if(diff ==1){
                    mArr[lastPoint] = monitor[i][j].mutex;
                    lastPoint++;
                }
            }
        }
    }

	//¤¿©±â±îÁö ÇÏ¸é ÃÑ ¸î°³ÀÇ mutexÁ¾·ù°¡ ÀÖ´Â Áö ¾Ë ¼ö ÀÖ´Ù.
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
   // int adjArray[count][count];
    
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
        if(monitor[i][j+1].mutex == NULL)    break;//a->b Á¤º¸°¡ ÇÊ¿äÇÏ±â ¶§¹®¿¡ ´ÙÀ½ ÀÎµ¦½º Á¤º¸ÀÎ j+1ÀÇ À¯¹«µµ È®ÀÎÇØ¾ßÇÔ.
        else{   
                //src->dest¿¡ °ü·Ã ÀÎµ¦½º¸¦ Ã£À½
                int src = getNum(monitor[i][j].mutex, count);
                int dest = getNum(monitor[i][j+1].mutex, count);
                //adjArray¿¡ Ç¥½Ã
                adjArray[src][dest] = 1;
            }
        }
    }
    
   
    return;
}
//find cycle
int cycleFinder(int* checker,int i, int count,int index){
    checker[i] = index;
    int j=0;
    for(j=0; j<count; j++){
        if(adjArray[i][j] == 1){
            if(checker[j] == 1){
               /* printf("|");
                for (int j=0; j<count; j++){
                    printf("%d ", checker[j]);
                }
                printf("|");*/
                return index-1;
            }
            else{
		index++;
                return cycleFinder(checker, j,count,index);
            }
            
        }
    }
    if(j==count){
	return 0;
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
		index  = arr[index];
		if(check[index] != 0)	break;
	}
	return mutexNode;
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

void adjPrinter(int** adjList){
	int count = mutexArray();
    printf("adjPrinter count : %d\n", count);
	for(int i=0; i<count; i++){
		for(int j=0; j<count; j++){
			printf("%d ",adjList[i][j]);
		}
		printf("\n");
	}
}



int
pthread_mutex_lock (pthread_mutex_t *mutex)
{
	static __thread int n_mutex = 0 ; //https://gcc.gnu.org/onlinedocs/gcc-3.3/gcc/Thread-Local.html
	n_mutex += 1 ;

	int  (*lockp)(pthread_mutex_t *mutex) ;
        char * error ;

        lockp = dlsym(RTLD_NEXT, "pthread_mutex_lock") ;
        if ((error = dlerror()) != 0x0)
                exit(1); 

	if (n_mutex == 1) {
		//add to monitor array
		addToMonitor(pthread_self(), mutex);
        	MakeAdjArray();
        	//adjPrinter(adjArray);
        	int count =mutexArray();
       		int checker[count];
        	int ans[count];
		for(int i=0; i<count; i++){
        	    checker[i] = 0;
	        }
		int index=cycleFinder(checker,0,count,0);		
		if(index){
			//»çÀÌÅ¬ÀÌ ÀÖ´Ù¸é
			int cycledMutex = countMutex(checker,index,count);
			printf("%d",cycledMutex);
		}
		
		int i ;
		void * arr[10] ;
		char ** stack ;


		size_t sz = backtrace(arr, 10) ;
		stack = backtrace_symbols(arr, sz) ;
		/*
		fprintf(stderr, "Stack trace\n") ;
		fprintf(stderr, "============\n") ;
		for (i = 0 ; i < sz ; i++)
			fprintf(stderr, "[%d] %s\n", i, stack[i]) ;
		fprintf(stderr, "============\n\n") ;
		*/
		lockp(mutex);
		}
    	
	n_mutex-= 1 ;

	return 0;
}
