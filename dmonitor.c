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
                //thid�� ������ ������ ĭ�� ã�Ƽ� �־��ָ� �ȴ�.
                if(monitor[i][j].thid == 0){//������ ĭ �߰�
                    monitor[i][j].thid = thid;
                    monitor[i][j].mutex = mutex;
                    return;
                }        
            }
        }else if(monitor[i][0].thid == 0){
            //���� thid�� ������ -> ���Ӱ� ����
            monitor[i][0].thid = thid;
            monitor[i][0].mutex = mutex;
            return;
        }
    }
}

//now have to find graph adjArray
int mutexArray(){
    
    //mutex array �ȿ� ���� �ٸ� mutex�� �־������.
    int lastPoint=0;
    //�迭�� �ݺ����� �����µ� tid=0�� ���������� �� ����
    for(int i=0 ;i<threadNum; i++){
	if(monitor[i][0].thid == 0){
		return lastPoint;
	}
	for(int j=0; j<mutexNum; j++){
            if(monitor[i][j].thid == 0)    break;
            else{
                int diff =1;// ���ο� ������ �ƴ� �������� �˷��ִ� ����-> ���Ӵٰ� �ʱ�ȭ
                for(int k=0; k<lastPoint; k++){//������ mutex�� ������ �ִ� ������ �˻�.
                    if(mArr[j]!=NULL && monitor[i][j].mutex == mArr[k])//������ �ʴ�-> diff=0 ���� ����
                        diff=0;
                }
                if(diff ==1){
                    mArr[lastPoint] = monitor[i][j].mutex;
                    lastPoint++;
                }
            }
        }
    }

	//�������� �ϸ� �� ��� mutex������ �ִ� �� �� �� �ִ�.
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
    //createArray();
    
    int count = mutexArray();
    int adjArray[count][count];
   for(int i=0; i<count; i++){
	for(int j=0; j<count; j++){
		adjArray[i][j] =0;
	}
} 
    for(int i=0 ;i<threadNum; i++){
    if(monitor[i][0].thid == 0){
        for(int i=0; i<count; i++){
            for(int j=0; j<count; j++){
                printf("%d ", adjArray[i][j]);
            }
            printf("\n");
        }
            return;
    }
    for(int j=0; j<mutexNum; j++){
        if(monitor[i][j+1].mutex == NULL)    break;//a->b ������ �ʿ��ϱ� ������ ���� �ε��� ������ j+1�� ������ Ȯ���ؾ���.
        else{   
                //src->dest�� ���� �ε����� ã��
                int src = getNum(monitor[i][j].mutex, count);
                int dest = getNum(monitor[i][j+1].mutex, count);
                //adjArray�� ǥ��
            printf("src : %d, dest %d\n", src, dest);
                adjArray[src][dest] = 1;
            }
        }
    }
    
   
    return;
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
		//printer();
        MakeAdjArray();
//        adjPrinter(MakeAdjArray());
		//adjArray();
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
