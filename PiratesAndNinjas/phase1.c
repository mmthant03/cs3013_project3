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
int avgNTime = 0;
int avgPTime = 0;
int avgNArrival = 0;
int avgPArrival = 0;
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
	return fabs(input + z);
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
		double randNo = spendTime(avgNTime);
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
				turnsRemainingRed = count; //set the next group of turns to 3
				for (int i = 0; i < count; i++)
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
				turnsRemainingBlue = count; //set the next group of turns to 3
				for (int i = 0; i < count; i++)
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
		//printf("blue lock released\n");

		sem_post(&mutex_blue);
		sem_post(&threeBluesMax);
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
		double randNo = spendTime(avgPTime);
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
				turnsRemainingBlue = count; //set the next group of turns to 3
				for (int i = 0; i < count; i++)
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
				turnsRemainingRed = count; //set the next group of turns to 3
				for (int i = 0; i < count; i++)
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

		//printf("red lock released\n");
		sem_post(&mutex_red);
		sem_post(&threeRedsMax);
		
	}
}
int threadList[175];

void printStat()
{
	for (int i = 1; i < 500; i++)
	{
		if (ourVisitData[i] == NULL)
		{
			break;
		}
		else
		{
			int currID = ourVisitData[i]->charID;
			char *type = ourVisitData[i]->typeChar;
			double goldSpent = ourVisitData[i]->goldSpent;
			int visitNum = ourVisitData[i]->visitNum;

			printf("\t----	Statistic ----\t\n");

			if (type == "P")
			{
				printf("Pirate Thread %d has total gold spent : %lf\t\t&\ttotal visits : %d\n", currID, goldSpent, visitNum);
				//printf("Pirate Thread %d has total gold spent : %lf\t&\ttotal visits : \n", currID, goldSpent);
			}
			else
			{
				printf("Ninja Thread %d has total gold spent : %lf\t\t&\ttotal visits : %d\n", currID, goldSpent, visitNum);
				//printf("Ninja Thread %d has total gold spent : %lf\t&\ttotal visits : \n", currID, goldSpent);
			}
			printf("\n");
		}
	}
}

// 0 means Null and 1 means not null
int isNULL()
{
	int notNull = 0;
	for (int i = 1; i < 100; i++)
	{
		if (threadList[i] != 0)
		{
			notNull = 1;
			break;
		}
		else
		{
			notNull = 0;
		}
	}
	return notNull;
}

void joinThread(pthread_t *pt)
{
	for (int i = 1; i < 175; i++)
	{
		if (threadList[0] == 0)
		{
			joinThread(pt);
		}
		else if (i == 174)
		{
			printf("Threads joined \n");
			break;
		}
		else
		{
			pthread_join(pt[i], NULL);
		}
		// else if (isNULL() == 1)
		// {
		// 	if (threadList[i] != 0)
		// 	{
		// 		pthread_join(pt[threadList[i]], NULL);
		// 		printf("Thread %d is joined\n", threadList[i]);
		// 	}
		// 	else
		// 	{
		// 		break;
		// 	}
		// }
	}
}

int createThread(int numNinja, int numPirate, int threadNum, int visitNum, int prevType, pthread_t *pt)
{
	threadArguments *arg = (threadArguments *)malloc(sizeof(threadArguments));
	arg->charID = threadNum;
	int type;
	if (numNinja <= 0 && numPirate <= 0)
	{
		return threadNum;
	}

	if (prevType == -1)
	{
		if (numNinja > 0 && numPirate > 0)
		{
			type = rand() % 2;
		}
		else if (numNinja <= 0 && numPirate > 0)
		{
			type = 0;
		}
		else if (numNinja > 0 && numPirate <= 0)
		{
			type = 1;
		}
	}
	else
	{
		type = prevType;
	}

	if (type == 0)
	{
		arg->typeChar = "P";
		numPirate--;
		printf("Pirate Thread created with Thread No : %d \n", threadNum);
	}
	else if (type == 1)
	{
		arg->typeChar = "N";
		numNinja--;
		printf("Ninja Thread created with Thread No : %d \n", threadNum);
	}
	arg->visitNum = visitNum;
	pthread_create(&pt[threadNum], NULL, thread, arg);

	threadList[0] = 1;

	for (int i = 1; i < 100; i++)
	{
		if (threadList[i] == threadNum)
		{
			break;
		}
		else if (threadList[i] == 0)
		{
			threadList[i] = threadNum;
			break;
		}
	}
	printf("Ninjas : %d \t&\t Pirates : %d \n", numNinja, numPirate);
	int willRevisit = rand25();

	if (numNinja <= 0 && numPirate <= 0)
	{
		return threadNum;
		//joinThread(pt);
	}
	else
	{
		// 1 means not going to revisit, 0 means thread is going to revisit
		if (willRevisit == 1)
		{
			if (numNinja <= 0 && numPirate > 0)
			{
				//pthread_join(pt[threadNum], NULL);
				visitNum = 1;
				threadNum = threadNum + 1;
				return createThread(numNinja, numPirate, threadNum, visitNum, 0, pt);
			}
			else if (numPirate <= 0 && numNinja > 0)
			{
				//pthread_join(pt[threadNum], NULL);
				visitNum = 1;
				threadNum = threadNum + 1;
				return createThread(numNinja, numPirate, threadNum, visitNum, 1, pt);
			}
			else
			{
				visitNum = 1;
				threadNum = threadNum + 1;
				return createThread(numNinja, numPirate, threadNum, visitNum, -1, pt);
			}
		}
		else
		{
			pthread_join(pt[threadNum], NULL);
			if (type == 0)
			{
				visitNum = visitNum + 1;
				//pthread_join(pt[threadNum], NULL);
				numPirate = numPirate + 1;
				printf("Pirate Thread %d is revisiting\n", threadNum);
				return createThread(numNinja, numPirate, threadNum, visitNum, type, pt);
			}
			else
			{
				visitNum = visitNum + 1;
				//pthread_join(pt[threadNum], NULL);
				numNinja = numNinja + 1;
				printf("Ninja Thread %d is revisiting\n", threadNum);
				return createThread(numNinja, numPirate, threadNum, visitNum, type, pt);
			}
		}
	}
}

int main(int argc, char *argv[])
{
	srand(time(NULL));
	//check the arguments

	// (WE WILL CHECK ARGUMENTS WHEN WE FINISH)
	int numRooms = 3;  //number of costuming teams (2-4)
	int numNinja = 6;  // arguments we will get from user
	int numPirate = 6; // arguments we will get from user

	if (argc == 7)
	{
		printf("Insufficient Arguments. Please provide exactly 7!\n\n");
		exit(1);
	}
	//number of costuming teams
	count = atoi(argv[1]);
	//number of pirates
	numPirate = atoi(argv[2]);
	//number of ninjas
	numNinja = atoi(argv[3]);
	//avg costume time for pirates
	avgPTime = atoi(argv[4]);
	//avg costime time for ninjas
	avgNTime = atoi(argv[5]);
	//if average costuming time of Pirate is shorter than that of Ninja
	if(avgPTime<avgNTime) {
		printf("Pirates tend to take longer than Ninjas. Please provide greater average time for Pirates!\n\n");
		exit(1);
	}
	//avg arrival time for pirates
	avgPArrival = atoi(argv[6]);
	//avg arrival time for ninjas
	avgNArrival = atoi(argv[7]);
	//initialize the semaphore
	sem_init(&numRedsFree, 0, numRooms);
	sem_init(&numBluesFree, 0, numRooms);
	sem_init(&redsTurn, 0, 0);
	sem_init(&mutex_blue, 0, 1);
	sem_init(&mutex_red, 0, 1);
	sem_init(&wrt, 0, 1);
	sem_init(&bluesTurn, 0, 0);
	sem_init(&threeBluesMax, 0, count); // arguments we will get from user
	sem_init(&threeRedsMax, 0, count);  // arguments we will get from user
	bluec = 0;
	redc = 0;
	turnsRemainingRed = count;
	turnsRemainingBlue = count;
	waitingBlues = 0;
	waitingReds = 0;
	//start up some threads
	pthread_t *pt = malloc(500 * sizeof(pthread_t));
	int threadNum = 1;
	int visitNum = 1;
	int lastThread = createThread(numNinja, numPirate, threadNum, visitNum, -1, pt);
	sleep(8);
	printf("\n\nLoading Statistic ...\n\n");
	sleep(3);
	printStat();
	sleep(5);
	joinThread(pt);
	sem_destroy(&numRedsFree);
	sem_destroy(&redsTurn);
	sem_destroy(&bluesTurn);
	sem_destroy(&numBluesFree);
	return 0;
}