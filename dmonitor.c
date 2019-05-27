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
                //thid는 같을때 마지막 칸을 찾아서 넣어주면 된다.
                if(monitor[i][j].thid == 0){//마지막 칸 발견
                    monitor[i][j].thid = thid;
                    monitor[i][j].mutex = mutex;
                    return;
                }        
            }
        }else if(monitor[i][0].thid == 0){
            //같은 thid가 없을때 -> 새롭게 시작
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
    
    //mutex array 안에 서로 다른 mutex를 넣어줘야함.
    int lastPoint=0;
    //배열을 반복문을 돌리는데 tid=0을 만나지않을 때 까지
    for(int i=0 ;i<threadNum; i++){
        if(monitor[i][0].thid == 0){
            return lastPoint;
        }
	for(int j=0; j<mutexNum; j++){
            if(monitor[i][j].thid == 0)    break;
            else{
                int diff =1;// 새로운 것인지 아닌 것인지를 알려주는 변수-> 새롭다고 초기화
                for(int k=0; k<lastPoint; k++){//지금의 mutex가 기존에 있는 것인지 검사.
                    if(mArr[j]!=NULL && monitor[i][j].mutex == mArr[k])//새롭지 않다-> diff=0 으로 변경
                        diff=0;
                }
                if(diff ==1){
                    mArr[lastPoint] = monitor[i][j].mutex;
                    lastPoint++;
                }
            }
        }
    }

	//ㅏ㈀瘦沮� 하면 총 몇개의 mutex종류가 있는 지 알 수 있다.
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
        if(monitor[i][j+1].mutex == NULL)    break;//a->b 정보가 필요하기 때문에 다음 인덱스 정보인 j+1의 유무도 확인해야함.
        else{   
                //src->dest에 관련 인덱스를 찾음
                int src = getNum(monitor[i][j].mutex, count);
                int dest = getNum(monitor[i][j+1].mutex, count);
                //adjArray에 표시
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
                return index;
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
	
		printf("cycle : %d\n",cycleFinder(checker,0,count,0));
		printf("checked %d ",checker[2]);
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
