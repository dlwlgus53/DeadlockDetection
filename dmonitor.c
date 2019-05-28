#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#define mutexNum 100
#define threadNum 10

struct Mnode//node for monitor
{       
        pthread_t thid; // initialized 0 
        pthread_mutex_t *mutex;
};
struct Edge
{
	pthread_t thid;
	pthread_mutex_t *src;
	pthread_mutex_t *dest;
};
struct thEdge
{
 	pthread_t src; 
	pthread_t dest;
};


pthread_mutex_t localmutex = PTHREAD_MUTEX_INITIALIZER;
struct Mnode monitor[threadNum][mutexNum];
pthread_mutex_t*mArr[mutexNum];
int ** adjArray;
//thread array(to see hierachy of thread)
struct thEdge thEdges[threadNum];

 

//segfault가 발생했을 때 호출될 함수

void sighandler(int sig)
{
    void *array[10];
    size_t size;
    char **strings;
    size_t i;

 

    size = backtrace(array, 10);

    strings = backtrace_symbols(array, size);

 

    //3. 앞에서 터득한 backtrace 정보 출력. 단 sighandler 스택 프레임은 제외하기 위해 2번 프레임부터 출력합니다.

    for(i = 2; i < size; i++)

        printf("%d: %s\n", i - 2, strings[i]);

 

    free(strings);

 

    exit(1);
}


void addToMonitor( pthread_t thid,pthread_mutex_t *mutex){//add to monitor array
    for(int i=0; i<threadNum; i++){
        if(monitor[i][0].thid == thid){
            for(int j=0; j<mutexNum; j++){
                if(monitor[i][j].mutex == NULL){//������ ĭ �߰�
                    monitor[i][j].thid = thid;
                    monitor[i][j].mutex = mutex;
                    return;
                }        
            }
        }else if(monitor[i][0].mutex == NULL){
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
    
    int lastPoint=0;
    for(int i=0 ;i<threadNum; i++){
        if(monitor[i][0].mutex == NULL){
            return lastPoint;
        }
	for(int j=0; j<mutexNum; j++){
            if(monitor[i][j].mutex == NULL)    break;
            else{
                int diff =1;// ���ο� ������ �ƴ� �������� �˷��ִ� ����-> ���Ӵٰ� �ʱ�ȭ
                for(int k=0; k<lastPoint; k++){//������ mutex�� ������ �ִ� ������ �˻�.
                    if( monitor[i][j].mutex == mArr[k])//������ �ʴ�-> diff=0 ���� ����
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
        if(monitor[i][j+1].mutex == NULL)    break;//a->b ������ �ʿ��ϱ� ������ ���� �ε��� ������ j+1�� ������ Ȯ���ؾ���.
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
 	printf("i %d  count  %d\n",i, count);  
int j=0;
    for(j=0; j<count; j++){
        if(adjArray[i][j] == 1){
            if(checker[j] == 1){
                printf("|");
                for (int j=0; j<count; j++){
                    printf("%d ", checker[j]);
                }
                printf("|");
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
		printf("index %d ",index);
		check[index]=1;
		mutexNode++;
		index = arr[index];
		if(check[index] != 0)	break;
	}
	return mutexNode;
}
pthread_t
findThid(pthread_mutex_t *src, pthread_mutex_t *dest){
	for(int i=0 ;i<threadNum; i++){
        if(monitor[i][0].thid == 0){
		printf("find error..\n");
            	return -1;
        }
        for(int j=0; j<mutexNum; j++){
            if(monitor[i][j+1].thid == 0)    break;
            else{
			if(monitor[i][j].mutex == src && monitor[i][j+1].mutex == dest){
				printf("index %d %d\n",i,j);
				printf("ihey: %lu",monitor[i][j].thid);
				return monitor[i][j].thid;
			} 
		}
	    }
        }
	return -1;
}
void
fillEdges(int* checker, int index, struct Edge* edges,int cycledMutex){
	//a->b, b->c��� ������ �ְ� ������	
	//1->2	�� a->b�� �ٲ���
	for(int i=0; i<cycledMutex; i++){
		int srcNum = checker[index];
		int destNum = checker[srcNum];

       		pthread_mutex_t *src = mArr[srcNum];
        	pthread_mutex_t *dest = mArr[destNum];

		//find a-b in array
		edges[i].thid  = findThid(src, dest);
		//add to edges
		edges[i].src = src;
		edges[i].dest = dest;

		index = checker[destNum];
	}
	printf("cycled mutex %d\n", cycledMutex);
	printf("%lu %lu\n", monitor[0][0].thid, monitor[1][0].thid);
	for(int i=0; i<cycledMutex; i++){
		printf("%lu ", edges[i].thid);
	}
}
//���� �����忡 ��ִ��� �˻�
int check1(struct Edge edges[], int count){
	int diff =1;
	for(int i=0; i<count-1; i++){
		for(int j=i+1; j<count; j++){
			if(edges[i].thid == edges[j].thid)	diff=0;
			}
		}
	return diff;

}
//guard�� �ִ��� �˻�
int check2(struct Edge edges[],int count){
	return 1;
}
//
int check3(struct Edge edges[],int count){
	return 1;

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

int
pthread_mutex_lock (pthread_mutex_t *mutex)
{
	signal(SIGSEGV, sighandler); 

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
        	MakeAdjArray();
        	//adjPrinter(adjArray);
        	int count =mutexArray();
       		int checker[count];
        	
		for(int i=0; i<count; i++){
        	    checker[i] = 0;
	        }
		int index=cycleFinder(checker,0,count);		
		printer();
		if(index==!-1){
			int cycledMutex = countMutex(checker,index,count);//����Ŭ�� �ִ� ��� ��
			printf("%d", cycledMutex);
			struct Edge edges[cycledMutex];//������ �� �ִ� node�� ���� ������ �������
			fillEdges(checker,index,edges,cycledMutex);
			if(check1(edges, count)&&check2(edges, count)&&check3(edges, count)){
				printf("danger\n");
				int i ;
                		void * arr[10] ;
                		char ** stack ;


                		size_t sz = backtrace(arr, 10) ;
                		stack = backtrace_symbols(arr, sz) ;
                			
				FILE * fp;
   				/* open the file for writing*/
   				fp = fopen ("dmonitor.trace","w");
 
   				for(i = 0; i < sz;i++){		
       					fprintf (fp, "%s\n", stack[i]);
   				}
 
   				/* close the file*/  
   				fclose (fp);
			}
		}
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
			return;
		}
	}
}
	
/* pthread array name : thEgeds */

int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start)(void *), void *arg){
	signal(SIGSEGV, sighandler); 

	int return_value=0;
	static __thread int n_create = 0 ; //https://gcc.gnu.org/onlinedocs/gcc-3.3/gcc/Thread-Local.html
	n_create += 1 ;


	static int (*pthread_createp)(pthread_t *thread, const pthread_attr_t *attr, void *(*start)(void *), void *arg) ;
	char * error ;
	
	pthread_createp = dlsym(RTLD_NEXT, "pthread_create") ;
	if ((error = dlerror()) != 0x0) 
		exit(1) ;
	

	if (n_create == 1) {
		
		
	//pthread_mutex_lock(&localmutex);
		return_value = pthread_createp(thread, attr, start, arg);
	
		pthread_t src = pthread_self();

	//	parameter : present thread, created thread
		addTothEdges(src, *thread);
		for(int i=0; i<threadNum; i++){
			if(thEdges[i].src == 0){
				printf("\n"); break;
		}			
				printf("%lu->%lu ",thEdges[i].src, thEdges[i].dest);
		}
		
 	//pthread_mutex_unlock(&localmutex);
	}

	n_create -= 1 ;
	return return_value ; 
}

