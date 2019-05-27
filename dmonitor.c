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
        pthread_t thid; // initialized 0 
        pthread_mutex_t *mutex;
};
struct Edge
{
	pthread_t thid;
	pthread_mutex_t *src;
	pthread_mutex_t *dest;
};

pthread_mutex_t localmutex = PTHREAD_MUTEX_INITIALIZER;
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
                //src->dest�� ���� �ε����� ã��
                int src = getNum(monitor[i][j].mutex, count);
                int dest = getNum(monitor[i][j+1].mutex, count);
                //adjArray�� ǥ��
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
                printf("|");
                for (int j=0; j<count; j++){
                    printf("%d ", checker[j]);
                }
                printf("|");
                return 1;
            }
            else{
		checker[i] = j;
                return cycleFinder(checker, j,count);
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
		int thid = findThid(src, dest);
		//add to edges
		edges[i].src = src;
		edges[i].dest = dest;
		edges[i].thid = thid;

		index = destNum;
		printf("destNum : %d\n", destNum);
	}
	printf("cycled mutex %d\n", cycledMutex);
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
	//1 -> �����ϴ� 0 -> �������� �ʴ�
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
		if(index){
			//����Ŭ�� �ִٸ�
			int cycledMutex = countMutex(checker,index,count);//����Ŭ�� �ִ� ��� ��
			printf("%d", cycledMutex);
			struct Edge edges[cycledMutex];//������ �� �ִ� node�� ���� ������ �������
			fillEdges(checker,index,edges,cycledMutex);
			/*edges �ȿ��� mutex* src, mutex* dest, thid ������ �ִ� �迭, count = ������ �Ǵ� node ��*/ 
			if(check1(edges, count)&&check2(edges, count)&&check3(edges, count)){
				//������ ����Ŭ���� ����-> backtrace ȣ��
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
