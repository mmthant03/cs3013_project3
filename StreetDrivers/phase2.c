#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <errno.h>


typedef struct threadArguments {
	int start;
	int end;
} threadArguments;


pthread_t tid[2]; 
int counter; 
pthread_mutex_t lock; 
int NW=0;
int SW=0;
int NE=0;
int SE=0;
  
void handleWhiteboardRelease(int start, int end) {
	//note this function is only called within a locked section so this will always be concurrent
	
	if (start == 6 && end == 3) {
		SE = 0;
	}
	
	
	
	
	
	
	
	
	
	
}
  
  
  
int handleWhiteboard(int start, int end) {
	//note this function is only called within a locked section so this will always be concurrent
	
	if (start == 6 && end == 3) {
		if (SE == 0) {
			SE = 1;
			return 1;
		}
	}
	
	
	
	
	
	
	
	
	
	//a square was taken; fails whiteboard
	sleep (0.1); //reduce the amount of spinning slightly, hopefully to improve performance
	return 0;
}
	
  
void* thread(void *arg) 
{ 
	//right is 3
	//down is 6
	int threadNum = 0;
	int start = 6;
	int end = 3;
	
	pthread_mutex_lock(&lock); 
	counter += 1; //just to keep track of threads, nothing more
	threadNum = counter; //just in case some other thread updates counter
	pthread_mutex_unlock(&lock); 
	
	int locksAcquired = 0;
	while (locksAcquired == 0) //spin until we can go
	{
		//try to claim whiteboard squares
		pthread_mutex_lock(&lock); 
		//printf("inside thread %d s1\n", threadNum);
		
		//try to claim our square
		locksAcquired = handleWhiteboard(start, end);

		
		pthread_mutex_unlock(&lock); 
	}
	
	//run the intersection
	printf("Thread %d has started at %d\n", threadNum, start); 
    sleep(1);
    printf("Thread %d has finished at %d\n", threadNum, end);
	
	
	//announce completion by releasing all held whiteboard pieces
	pthread_mutex_lock(&lock); 
	printf("inside thread %d s2 releasing squares\n", threadNum);
	//we know we're going right in this example
	handleWhiteboardRelease(start, end);
	pthread_mutex_unlock(&lock); 
  
    return NULL; 
}



int main(int argc, char* argv[]) {


	/*
	threadArguments* arg = (threadArguments*)malloc(sizeof(threadArguments));
	arg ->charID = 1;
	arg ->typeChar = "P";
	arg -> visitNum = 1;
    //pthread_create(&t1,NULL,thread, arg); 
	
	threadArguments* arg2 = (threadArguments*)malloc(sizeof(threadArguments));
	arg2 ->charID = 1;
	arg2 ->typeChar = "N";
	arg2 -> visitNum = 1;
    //pthread_create(&t2,NULL,thread, arg2); 
	*/
	//copy
	int i = 0; 
    int error; 
	if (pthread_mutex_init(&lock, NULL) != 0) 
    { 
        printf("\n mutex init has failed\n"); 
        return 1; 
    } 
  

	pthread_create(&(tid[1]), NULL, &thread, NULL);
	pthread_create(&(tid[2]), NULL, &thread, NULL);
	sleep(4);
    pthread_join(tid[0], NULL); 
    pthread_join(tid[1], NULL); 
    pthread_mutex_destroy(&lock); 
  
    return 0; 
	//end copied section
	
	
}