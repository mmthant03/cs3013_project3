#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <math.h>
#include <time.h>

#define PI 3.14159265358979323846

typedef struct visitData
{
	int charID;		//1, 2 just to uniquely identify each ninja or pirate
	char *typeChar; //"N" for ninja, "P" for pirate
	double waitTime;
	double spentTime;
	double goldSpent;
	//this will hold the number of visits
	//do note that if this is >1, the second visit will be recorded in a separate visitData with the matching charID
	//where the first will retain visitNum = 1;
	int visitNum;
} visitData;

typedef struct threadArguments
{
	int charID;
	char *typeChar;
	int visitNum;
} threadArguments;

sem_t bluesTurn;
sem_t redsTurn;
sem_t numRedsFree;
sem_t numBluesFree;
sem_t mutex_blue;
sem_t mutex_red;
sem_t wrt;
int bluec;
int redc;
int turnsRemainingBlue;
int turnsRemainingRed;
int waitingBlues;
int waitingReds;
int count = 0;
sem_t threeBluesMax;
sem_t threeRedsMax;
visitData *ourVisitData[500];

int rand50()
{
	return rand() & 1;
}

int rand25()
{
	return rand50() | rand50();
}

double boxMuller(double a, double b)
{
	return sqrt(-2 * log(a)) * cos(2 * PI * b);
}

double spendTime(int avgTime)
{
	double drand1 = (rand() + 1.0) / (RAND_MAX + 1.0);
	double drand2 = (rand() + 1.0) / (RAND_MAX + 1.0);
	double input = avgTime + 0.00;
	double z = boxMuller(drand1, drand2);
	return input + z;
}

/*for either ninja or pirate, the structure goes as follows:
	- try to be one of the first three ninjas or pirates in the ready state (3 ninjas and 3 pirates can hold threeBluesMax and threeRedsMax)
	- if they are one of the 3 at the front of the line, they try to enter the shop
		- they cannot enter the shop if the other team is in the shop (holds their team's lock)
	- if they get into the shop, they get their costume built
	- when the last member exits and the shop will be empty, each team checks if the other team is waiting, if so it gives the thread to the other team and lets three go
*/
void *thread(void *arg)
{
	//acquire all our variables
	threadArguments thisArg = *((threadArguments *)arg);
	int charID = thisArg.charID;
	char *typeChar = thisArg.typeChar;
	int visitNum = thisArg.visitNum;
	//we passed in all our fields
	//TODO @MYO
	//typeChar is just N or P, we use that already
	//charID is what they'll be stored into our visitData struct as, that's why we have it
	//visitNum is always 1 to start
	//visitNum will be visitNum++ when the 25% chance to re-visit goes again
	//at which point, you call thread again

	//if Ninja
	if (typeChar == "N")
	{ //if blue

		visitData *vD = (visitData *)malloc(sizeof(visitData));
		vD->charID = charID;
		vD->typeChar = "N";
		vD->visitNum = visitNum;

		//try to be one of the three blues that exist
		sem_wait(&threeBluesMax);

		//check if the 3 blue turns are taken
		//if so, sleep until a red wakes you up specifically
		if (turnsRemainingBlue <= 0)
		{
			waitingBlues++;
			sem_wait(&bluesTurn);
		}

		//enter & start waiting
		clock_t startTimeWait = clock();

		//printf("blue TRYINg to enter\n");
		sem_wait(&mutex_blue);
		//printf("blue lock acquired = blue 1\n");
		bluec++;
		turnsRemainingBlue--;
		if (bluec == 1)
		{
			sem_wait(&mutex_red); //lock out all reds
								  //printf("red lock acquired = blue 1\n");
		}
		sem_post(&mutex_blue);
		//printf("blue lock released\n");

		//stop waiting
		clock_t endTimeWait = clock();
		double waitTime = (double)(endTimeWait - startTimeWait) / CLOCKS_PER_SEC;
		vD->waitTime = waitTime;

		//create the costume
		printf("NINJA THREAD %d ENTERS\n", charID);

		//generate time to spend
		double randNo = spendTime(2);
		int timeSpentN = (int)((randNo)*60.00);
		//spend time and save the time spent
		sleep(1);
		vD->spentTime = randNo;

		//calculate gold
		if (waitTime > 30.00)
		{
			vD->goldSpent = vD->goldSpent + 0.0;
		}
		else
		{
			vD->goldSpent = vD->goldSpent + randNo;
		}
		ourVisitData[charID] = vD;
		printf("NINJA %d LEAVES\n", charID);

		//LEAVE
		sem_wait(&mutex_blue);
		//printf("blue lock acquired = blue 2\n");
		bluec--;
		if (bluec == 0)
		{ //if the last blue is exiting
			//THIS SECTION HANDLES THE NEXT TURN;
			//IF there is a red waiting, let red go
			//if there is no red let blue go
			if (waitingReds > 0)
			{
				turnsRemainingRed = 3; //set the next group of turns to 3
				for (int i = 0; i < 3; i++)
				{
					if (waitingReds > 0)
					{
						//printf("trying to wake up a red\n");
						sem_post(&redsTurn);
						waitingReds--;
					}
				}
			}
			else
			{
				turnsRemainingBlue = 3; //set the next group of turns to 3
				for (int i = 0; i < 3; i++)
				{
					if (waitingBlues > 0)
					{
						//printf("trying to wake up a blue\n");
						sem_post(&bluesTurn);
						waitingBlues--;
					}
				}
			}
			sem_post(&mutex_red);
			//printf("red lock released\n");
		}
		sem_post(&mutex_blue);
		//printf("blue lock released\n");

		sem_post(&threeBluesMax);

		//when the thread ends
		//add this data into array to keep track of stat
		//ourVisitData[charID] = vD;

		// int willRevisit = rand25();
		// if(willRevisit == 0) {
		// 	count++;
		// 	printf("Revisiting Ninja Thread %d\n", charID);
		// 	thisArg.visitNum++;
		// 	threadArguments* arg = (threadArguments*)malloc(sizeof(threadArguments));
		// 	arg ->charID = thisArg.charID;
		// 	arg ->typeChar = thisArg.typeChar;
		// 	arg -> visitNum = thisArg.visitNum + 1;
		// 	thread(arg);
		// }
	}

	//if Pirate
	else if (typeChar == "P")
	{ //if red

		visitData *vD = (visitData *)malloc(sizeof(visitData));
		vD->charID = charID;
		vD->typeChar = "P";
		vD->visitNum = visitNum;

		//try to be one of the only 3 reds
		sem_wait(&threeRedsMax);

		//check if the 3 blue turns are taken
		//if so, sleep until a red turn goes again
		if (turnsRemainingRed <= 0)
		{
			waitingReds++;
			sem_wait(&redsTurn);
		}

		//enter & start waiting
		clock_t startTimeWait = clock();

		//printf("red TRYINg to enter\n");
		sem_wait(&mutex_red);
		//printf("red lock acquired = red 1\n");
		redc++;
		turnsRemainingRed--;
		if (redc == 1)
		{ //lock out all blues
			sem_wait(&mutex_blue);
			//printf("blue lock acquired = red 1\n");
		}
		sem_post(&mutex_red);
		//printf("red lock released\n");

		//stop waiting
		clock_t endTimeWait = clock();
		double waitTime = (double)(endTimeWait - startTimeWait) / CLOCKS_PER_SEC;
		vD->waitTime = waitTime;

		//costume time
		printf("PIRATES %d ENTERS\n", charID);

		//generate time to spend
		double randNo = spendTime(2);
		int timeSpentP = (int)((randNo)*60.00);
		//spend time and save the time spent
		sleep(1);
		vD->spentTime = randNo;

		//calculate gold
		if (waitTime > 30.00)
		{
			vD->goldSpent = vD->goldSpent + 0.0;
		}
		else
		{
			vD->goldSpent = vD->goldSpent + randNo;
		}
		ourVisitData[charID] = vD;
		//printf("Red Data %d\n", ourVisitData[charID]->visitNum);
		printf("PIRATE %d LEAVES\n", charID);

		//LEAVE
		sem_wait(&mutex_red);
		//printf("red lock acquired = red 2\n");
		redc--;
		if (redc == 0)
		{ //if the last red is out
			//THIS SECTION HANDLES THE NEXT TURN;
			//IF there is a blue waiting, let red go
			//if there is no blues waiting, let red go
			if (waitingReds > 0)
			{
				turnsRemainingBlue = 3; //set the next group of turns to 3
				for (int i = 0; i < 3; i++)
				{
					if (waitingBlues > 0)
					{
						//printf("trying to wake up a blue\n");
						sem_post(&bluesTurn);
						waitingBlues--;
					}
				}
			}
			else
			{
				turnsRemainingRed = 3; //set the next group of turns to 3
				for (int i = 0; i < 3; i++)
				{
					if (waitingReds > 0)
					{
						//printf("trying to wake up a red \n");
						sem_post(&redsTurn);
						waitingReds--;
					}
				}
			}
			sem_post(&mutex_blue);
			//printf("blue lock released\n");
		}
		sem_post(&mutex_red);
		//printf("red lock released\n");

		sem_post(&threeRedsMax);

		//when the thread ends
		//add this data into array to keep track of stat
		//ourVisitData[charID] = vD;

		// int willRevisit = rand25();
		// if(willRevisit == 0) {
		// 	count++;
		// 	printf("Revisting Pirate Thread %d\n", charID);
		// 	thisArg.visitNum++;
		// 	threadArguments* arg = (threadArguments*)malloc(sizeof(threadArguments));
		// 	arg ->charID = thisArg.charID;
		// 	arg ->typeChar = thisArg.typeChar;
		// 	arg -> visitNum = thisArg.visitNum + 1;
		// 	thread(arg);
		// }
	}
}
int threadList[100];

void printStat() {
	for (int i = 1; i < 500; i++) {
		if(ourVisitData[i] == NULL){
			break;
		} else {
			int currID = ourVisitData[i]->charID;
			char* type = ourVisitData[i]->typeChar;
			double goldSpent = ourVisitData[i]->goldSpent;

			printf("\t----	Statistic ----\t\n");
			
			if(type == "P") {
				printf("Pirate Thread %d has total gold spent : %lf\n", currID, goldSpent);
			} else {
				printf("Ninja Thread %d has total gold spent : %lf\n", currID, goldSpent);
			}
			printf("\n");
		}
	}
}

void joinThread(pthread_t *pt)
{
	for (int i = 0; i < 100; i++)
	{
		if (threadList[0] == 0)
		{
			joinThread(pt);
		}
		else
		{	
			pthread_join(pt[threadList[i]], NULL);
		}
	}
}



int createThread(int numNinja, int numPirate, int threadNum, int visitNum, pthread_t *pt)
{
	threadArguments *arg = (threadArguments *)malloc(sizeof(threadArguments));
	arg->charID = threadNum;
	int type;
	if(numNinja == 0 && numPirate == 0) {
		return threadNum;
		//joinThread(pt);
	} else if (numNinja != 0 && numPirate != 0) {
		type = rand() % 2;
	} else if (numNinja == 0 && numPirate != 0) {
		type = 0;
	} else {
		type = 1;
	}
	
	if (type == 0)
	{
		arg->typeChar = "P";
		numPirate--;
	}
	else if (type == 1)
	{
		arg->typeChar = "N";
		numNinja--;
	}
	arg->visitNum = visitNum;
	pthread_create(&pt[threadNum], NULL, thread, arg);
	for(int i = 0; i < 100; i++) {
		if(threadList[i] == threadNum){
			break;
		} else if (threadList[i] == 0) {
			threadList[i] = threadNum;
			break;
		}
	}
	printf("Ninjas : %d \t&\t Pirates : %d \n", numNinja, numPirate);
	int willRevisit = rand25();
	 if (numNinja == 0 && numPirate == 0)
	 {
	 	return threadNum;
		//joinThread(pt);
	 }
	 else
	 {
		if (willRevisit == 1)
		{
			visitNum = 1;
			threadNum = threadNum + 1;
			createThread(numNinja, numPirate, threadNum, visitNum, pt);
		}
		else
		{
			visitNum = visitNum + 1;
			pthread_join(pt[threadNum], NULL);
			createThread(numNinja, numPirate, threadNum, visitNum, pt);
		}
	}
}


int main(int argc, char *argv[])
{
	srand(time(0));
	//check the arguments
	// (WE WILL CHECK ARGUMENTS WHEN WE FINISH)
	int numRooms = 3; //number of costuming teams (2-4)

	//initialize the semaphore
	sem_init(&numRedsFree, 0, numRooms);
	sem_init(&numBluesFree, 0, numRooms);
	sem_init(&redsTurn, 0, 0);
	sem_init(&mutex_blue, 0, 1);
	sem_init(&mutex_red, 0, 1);
	sem_init(&wrt, 0, 1);
	sem_init(&bluesTurn, 0, 0);
	sem_init(&threeBluesMax, 0, 3); // arguments we will get from user
	sem_init(&threeRedsMax, 0, 3);  // arguments we will get from user
	bluec = 0;
	redc = 0;
	turnsRemainingRed = 3;
	turnsRemainingBlue = 3;
	waitingBlues = 0;
	waitingReds = 0;
	int numNinja = 6;  // arguments we will get from user
	int numPirate = 6; // arguments we will get from user

	//TODO @myo put thread creation into a loop please, with random times
	//start up some threads
	pthread_t *pt = malloc(500 * sizeof(pthread_t));
	int threadNum = 1;
	int visitNum = 1;
	int lastThread = createThread(numNinja, numPirate, threadNum, visitNum, pt);
	sleep(5);
	joinThread(pt);
	sleep(5);
	printStat();

	/*
	int totalthreads = numNinja + numPirate;
	int i = 1;
	while (i > 0) //totalthreads + 1)
	{
		int type;
		if (numNinja == 0 && numPirate == 0)
		{
			break;
		}
		else if (numNinja != 0 && numPirate != 0)
		{
			type = rand() % 2;
		}
		else if (numNinja == 0)
		{
			type = 0;
		}
		else
		{
			type = 1;
		}

		if (type == 0)
		{
			numPirate--;
		}
		else
		{
			numNinja--;
		}
		int visitNum = 1;
		char *visited = createThread(type, i, visitNum, pt);
		if (visited == "N")
		{
			numNinja++;
		}
		else if (visited == "P")
		{
			numPirate++;
		}
		i++;
		//i = i + visited;
		//totalthreads = totalthreads + visited;
	}
	printf("The total number of threads are %d\n", i);
	int j = 1;
	while (j < i + 1)
	{
		pthread_join(pt[j], NULL);
		j++;
	}


	/*
	while(i<totalthreads) {
		int randNP = rand()%2;
		threadArguments* arg = (threadArguments*)malloc(sizeof(threadArguments));
		arg ->charID = i;
		if (randNP == 0 && numPirate != 0) {
			arg ->typeChar = "P";
			numPirate--;
		} else if (randNP == 1 && numNinja != 0) {
			arg ->typeChar = "N";
			numNinja--;
		}
		arg -> visitNum = 1;
		pthread_create(&pt[i], NULL, thread, arg);
		i++;
		count++;
	}
	int j = 0;
	while(j<count) {
		pthread_join(pt[j], NULL);
		j++;
	}

	/*
    pthread_t t1,t2, t3, t4, t5, t6, t7, t8, t9; 
	//void* thread (int charID, char typeChar, int visitNum)
	threadArguments* arg = (threadArguments*)malloc(sizeof(threadArguments));
	arg ->charID = 1;
	arg ->typeChar = "P";
	arg -> visitNum = 1;
    pthread_create(&t1,NULL,thread, arg); 
	
	threadArguments* arg2 = (threadArguments*)malloc(sizeof(threadArguments));
	arg2 ->charID = 1;
	arg2 ->typeChar = "N";
	arg2 -> visitNum = 1;
    pthread_create(&t2,NULL,thread, arg2); 
	
    sleep(4); 
    pthread_join(t1,NULL); 
    pthread_join(t2,NULL);
	*/
	sem_destroy(&numRedsFree);
	sem_destroy(&redsTurn);
	sem_destroy(&bluesTurn);
	sem_destroy(&numBluesFree);
	return 0;
}