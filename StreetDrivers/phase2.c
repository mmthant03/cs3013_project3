#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <time.h>

typedef struct threadArguments
{
	int threadNum;
	int start;
	int end;
} threadArguments;

threadArguments *queues[20];

pthread_t tid[2];
int counter;
pthread_mutex_t lock;
int NW = 0;
int SW = 0;
int NE = 0;
int SE = 0;

/** Initialize the queue with 20 threads/cars
 * 	Each car has different start and end destionations
 *  12 -> N, 3 -> E, 6 -> South, 9 -> West
 */
void threadInit()
{
	srand(time(NULL));
	for (int i = 0; i < 20; i++)
	{
		threadArguments *thread = (threadArguments *)malloc(sizeof(threadArguments));
		if (queues[i] == NULL)
		{
			int startDirection = rand() % 4;		   // 0, 1, 2 or 3
			startDirection = (startDirection + 1) * 3; // 3, 6, 9 or 12
			int endDirection = rand() % 4;			   // 0, 1, 2 or 3
			endDirection = (endDirection + 1) * 3;	 // 3, 6, 9 or 12
			if (startDirection == endDirection)
			{
				endDirection = ((startDirection + 1) % 4) * 3;
				if (endDirection == 0)
				{
					endDirection = 12;
				}
				thread->start = startDirection;
				thread->end = endDirection;
			}
			else
			{
				thread->start = startDirection;
				thread->end = endDirection;
			}
		}
		queues[i] = thread;
	}
}

/** Pop the first in the queue according to direction
 * 	Will return the desired destination if found
 * 	If not found, will return 0 to indicate there is no more car in the queue
 */
int popThread(int start)
{
	for (int i = 0; i < 20; i++)
	{
		if (queues[i] != NULL)
		{
			int startInQueue = queues[i]->start;
			if (startInQueue == start)
			{
				return queues[i]->end;
				break;
			}
			else
			{
				return 0;
			}
		}
	}
}

void handleWhiteboardRelease(int start, int end)
{
	//note this function is only called within a locked section so this will always be concurrent

	if (start == 6 && end == 3)
	{
		SE = 0;
	}
	else if (start == 6 && end == 12)
	{
		SE = 0;
		NE = 0;
	}
	else if (start == 6 && end == 9)
	{
		SE = 0;
		NE = 0;
		NW = 0;
	}
	else if (start == 3 && end == 9)
	{
		NE = 0;
		NW = 0;
	}
	else if (start == 3 && end == 12)
	{
		NE = 0;
	}
	else if (start == 3 && end == 6)
	{
		NE = 0;
		NW = 0;
		SW = 0;
	}
	else if (start == 12 && end == 9)
	{
		NW = 0;
	}
	else if (start == 12 && end == 6)
	{
		NW = 0;
		SW = 0;
	}
	else if (start == 12 && end == 3)
	{
		NW = 0;
		SW = 0;
		SE = 0;
	}
	else if (start == 9 && end == 3)
	{
		SE = 0;
		SW = 0;
	}
	else if (start == 9 && end == 12)
	{
		SE = 0;
		SW = 0;
		NE = 0;
	}
	else if (start == 9 && end == 6)
	{
		SW = 0;
	}
}

int handleWhiteboard(int start, int end)
{
	//note this function is only called within a locked section so this will always be concurrent

	if (start == 6 && end == 3)
	{
		if (SE == 0)
		{
			SE = 1;
			return 1;
		}
	}
	else if (start == 6 && end == 12)
	{
		if (SE == 0 && NE == 0)
		{
			SE = 1;
			NE = 1;
			return 1;
		}
	}
	else if (start == 6 && end == 9)
	{
		if (SE == 0 && NE == 0 && NW == 0)
		{
			SE = 1;
			NE = 1;
			NW = 1;
			return 1;
		}
	}
	else if (start == 3 && end == 9)
	{
		if (NE == 0 && NW == 0)
		{
			NE = 1;
			NW = 1;
			return 1;
		}
	}
	else if (start == 3 && end == 12)
	{
		if (NE == 0)
		{
			NE = 1;
			return 1;
		}
	}
	else if (start == 3 && end == 6)
	{
		if (NE == 0 && NW == 0 && SW == 0)
		{
			NE = 1;
			SW = 1;
			NW = 1;
			return 1;
		}
	}
	else if (start == 12 && end == 9)
	{
		if (NW == 0)
		{
			NW = 1;
			return 1;
		}
	}
	else if (start == 12 && end == 6)
	{
		if (NW == 0 && SW == 0)
		{
			NW = 1;
			SW = 1;
			return 1;
		}
	}
	else if (start == 12 && end == 3)
	{
		if (NW == 0 && SW == 0 && SE == 0)
		{
			NW = 1;
			SW = 1;
			SE = 1;
			return 1;
		}
	}
	else if (start == 9 && end == 3)
	{
		if (SE == 0 && SW == 0)
		{
			SE = 1;
			SW = 1;
			return 1;
		}
	}
	else if (start == 9 && end == 12)
	{
		if (SE == 0 && SW == 0 && NE == 0)
		{
			SE = 1;
			SW = 1;
			NE = 1;
			return 1;
		}
	}
	else if (start == 9 && end == 6)
	{
		if (SW == 0)
		{
			SW = 1;
			return 1;
		}
	}

	//a square was taken; fails whiteboard
	sleep(0.1); //reduce the amount of spinning slightly, hopefully to improve performance
	return 0;
}

void *thread(void *arg)
{
	//right is 3
	//down is 6
	threadArguments thread = *((threadArguments *)arg);

	int threadNum = thread.threadNum;
	int start = thread.start;
	int end = thread.end;

	pthread_mutex_lock(&lock);
	counter += 1;		 //just to keep track of threads, nothing more
	//threadNum = counter; //just in case some other thread updates counter
	pthread_mutex_unlock(&lock);
	// if (threadNum == 2)
	// {
	// 	start = 12;
	// 	end = 9;
	// }
	// if (threadNum == 3)
	// {
	// 	start = 3;
	// 	end = 12;
	// }
	// if (threadNum == 4)
	// {
	// 	start = 9;
	// 	end = 6;
	// }

	int locksAcquired = 0;
	while (locksAcquired == 0) //spin until we can go
	{
		//try to claim whiteboard squares
		pthread_mutex_lock(&lock);
		printf("Thread %d is trying to acquire the squares (%d to %d)\n", threadNum, start, end);

		//try to claim our square
		locksAcquired = handleWhiteboard(start, end);
		if (locksAcquired == 1)
		{
			printf("THREAD %d ACQUIRED SQUARES!!\n", threadNum);
		}
		pthread_mutex_unlock(&lock);

		sleep(1);
	}

	//run the intersection
	printf("Thread %d has started at %d\n", threadNum, start);
	sleep(1);
	printf("Thread %d has finished at %d\n", threadNum, end);

	//announce completion by releasing all held whiteboard pieces
	pthread_mutex_lock(&lock);
	printf("Thread %d s2 releasing squares\n", threadNum);
	handleWhiteboardRelease(start, end);
	pthread_mutex_unlock(&lock);

	return NULL;
}

int main(int argc, char *argv[])
{
	srand(time(NULL));
	threadInit();
	for (int i = 0; i < 20; i++)
	{
		printf("Inserting into queue= %d / start= %d / end= %d \n", i, queues[i]->start, queues[i]->end);
	}
	printf("Queue populated with random start and end values !\n");
	pthread_t *pt = malloc(20 * sizeof(pthread_t));
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
	NE = 0;
	NW = 0;
	SE = 0;
	SW = 0;

	//copy

	//int i = 0;
	int error;
	printf("Creating lock\n");
	if (pthread_mutex_init(&lock, NULL) != 0)
	{
		printf("\n mutex init has failed\n");
		return 1;
	}

	// -1 means available
	int N = -1;
	int E = -1;
	int S = -1;
	int W = -1;
	
	printf("Starting up our first four cars, one for each position\n");
	for(int i = 0; i < 21; i++) {
		if (i == 20) {
			sleep(2);
			pthread_join(pt[N], NULL);
			pthread_join(pt[E], NULL);
			pthread_join(pt[S], NULL);
			pthread_join(pt[W], NULL);
			N = -1;
			E = -1;
			S = -1;
			W = -1;
			break;
		}
		if(queues[i] != NULL) {
			if(N == -1 && queues[i]->start == 12) {
				threadArguments *argN = (threadArguments *)malloc(sizeof(threadArguments));
				argN->threadNum = i;
				argN->start = queues[i]->start;
				argN->end = queues[i]->end;
				pthread_create(&pt[i], NULL, thread, argN);
				N = i;
				queues[i]->start = 0;
			} else if (N != -1 && queues[i]->start == 12) {
				sleep(2);
				pthread_join(pt[N], NULL);
				sleep(2);
				threadArguments *argN = (threadArguments *)malloc(sizeof(threadArguments));
				argN->threadNum = i;
				argN->start = queues[i]->start;
				argN->end = queues[i]->end;
				pthread_create(&pt[i], NULL, thread, argN);
				N = i;
				queues[i]->start = 0;
			}

			if(E == -1 && queues[i]->start == 3) {
				threadArguments *argE = (threadArguments *)malloc(sizeof(threadArguments));
				argE->threadNum = i;
				argE->start = queues[i]->start;
				argE->end = queues[i]->end;
				pthread_create(&pt[i], NULL, thread, argE);
				E = i;
				queues[i]->start = 0;
			} else if (E != -1 && queues[i]->start == 3) {
				sleep(2);
				pthread_join(pt[E], NULL);
				sleep(2);
				threadArguments *argE = (threadArguments *)malloc(sizeof(threadArguments));
				argE->threadNum = i;
				argE->start = queues[i]->start;
				argE->end = queues[i]->end;
				pthread_create(&pt[i], NULL, thread, argE);
				E = i;
				queues[i]->start = 0;
			}

			if(S == -1 && queues[i]->start == 6) {
				threadArguments *argS = (threadArguments *)malloc(sizeof(threadArguments));
				argS->threadNum = i;
				argS->start = queues[i]->start;
				argS->end = queues[i]->end;
				pthread_create(&pt[i], NULL, thread, argS);
				S = i;
				queues[i]->start = 0;
			} else if (S != -1 && queues[i]->start == 6) {
				sleep(2);
				pthread_join(pt[S], NULL);
				sleep(2);
				threadArguments *argS = (threadArguments *)malloc(sizeof(threadArguments));
				argS->threadNum = i;
				argS->start = queues[i]->start;
				argS->end = queues[i]->end;
				pthread_create(&pt[i], NULL, thread, argS);
				S = i;
				queues[i]->start = 0;
			}

			if(W == -1 && queues[i]->start == 9) {
				threadArguments *argW = (threadArguments *)malloc(sizeof(threadArguments));
				argW->threadNum = i;
				argW->start = queues[i]->start;
				argW->end = queues[i]->end;
				pthread_create(&pt[i], NULL, thread, argW);
				W = i;
				queues[i]->start = 0;
			} else if (W != -1 && queues[i]->start == 9) {
				sleep(2);
				pthread_join(pt[W], NULL);
				threadArguments *argW = (threadArguments *)malloc(sizeof(threadArguments));
				argW->threadNum = i;
				argW->start = queues[i]->start;
				argW->end = queues[i]->end;
				pthread_create(&pt[i], NULL, thread, argW);
				W = i;
				queues[i]->start = 0;
			}
		}
	}
	
	pthread_mutex_destroy(&lock);

	return 0;
	//end copied section
}