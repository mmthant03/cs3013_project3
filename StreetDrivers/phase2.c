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
		SE == 0;
	}
	else if (start == 6 && end == 12) {
		SE == 0;
		NE == 0;
	}
	else if (start == 6 && end == 9) {
		SE == 0;
		NE == 0;
		NW == 0;
	}
	else if (start == 3 && end == 9) {
		NE == 0;
		NW == 0;
	}
	else if (start == 3 && end == 12) {
		NE == 0;
	}
	else if (start == 3 && end == 6) {
		NE == 0;
		NW == 0;
		SW == 0;
	}
	else if (start == 12 && end == 9) {
		NW == 0;
	}
	else if (start == 12 && end == 6) {
		NW == 0;
		SW == 0;
	}
	else if (start == 12 && end == 3) {
		NW == 0;
		SW == 0;
		SE == 0;
	}
	else if (start == 9 && end == 3) {
		SE == 0;
		SW == 0;
	}
	else if (start == 9 && end == 12) {
		SE == 0;
		SW == 0;
		NE == 0;
	}
	else if (start == 9 && end == 6) {
		SW == 0;
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
	else if (start == 6 && end == 12) {
		if (SE == 0 && NE == 0) {
			SE = 1;
			NE = 1;
			return 1;
		}
	}
	else if (start == 6 && end == 9) {
		if (SE == 0 && NE == 0 && NW == 0) {
			SE = 1;
			NE = 1;
			NW = 1;
			return 1;
		}
	}
	else if (start == 3 && end == 9) {
		if (NE == 0 && NW == 0) {
			NE = 1;
			NW = 1;
			return 1;
		}
	}
	else if (start == 3 && end == 12) {
		if (NE == 0) {
			NE = 1;
			return 1;
		}
	}
	else if (start == 3 && end == 6) {
		if (NE == 0 && NW == 0 && SW == 0) {
			NE = 1;
			SW = 1;
			NW = 1;
			return 1;
		}
	}
	else if (start == 12 && end == 9) {
		if (NW == 0) {
			NW = 1;
			return 1;
		}
	}
	else if (start == 12 && end == 6) {
		if (NW == 0 && SW == 0) {
			NW = 1;
			SW = 1;
			return 1;
		}
	}
	else if (start == 12 && end == 3) {
		if (NW == 0 && SW == 0 && SE == 0) {
			NW = 1;
			SW = 1;
			SE = 1;
			return 1;
		}
	}
	else if (start == 9 && end == 3) {
		if (SE == 0 && SW == 0) {
			SE = 1;
			SW = 1;
			return 1;
		}
	}
	else if (start == 9 && end == 12) {
		if (SE == 0 && SW == 0 && NE == 0) {
			SE = 1;
			SW = 1;
			NE = 1;
			return 1;
		}
	}
	else if (start == 9 && end == 6) {
		if (SW == 0) {
			SW = 1;
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
	if (threadNum == 2) {
		start = 12;
		end = 9;
	}
	if (threadNum == 3) {
		start = 3;
		end = 12;
	}
	if (threadNum == 4) {
		start = 9;
		end = 6;
	}
	
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
	pthread_create(&(tid[3]), NULL, &thread, NULL);
	pthread_create(&(tid[4]), NULL, &thread, NULL);
	sleep(4);
    pthread_join(tid[0], NULL); 
    pthread_join(tid[1], NULL); 
    pthread_join(tid[2], NULL); 
    pthread_join(tid[3], NULL); 
    pthread_mutex_destroy(&lock); 
  
    return 0; 
	//end copied section
	
	
}