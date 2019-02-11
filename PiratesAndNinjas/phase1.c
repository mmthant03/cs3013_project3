#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

typedef struct visitData {
	int charID; //1, 2 just to uniquely identify each ninja or pirate
	char typeChar; //"N" for ninja, "P" for pirate
	clock_t startTimeWait;
	clock_t endTimeWait;
	clock_t startTimeCostum;
	clock_t endTimeCostume;
	//this will hold the number of visits
	//do note that if this is >1, the second visit will be recorded in a separate visitData with the matching charID
	//where the first will retain visitNum = 1;
	int visitNum;
} visitData;

typedef struct threadArguments {
	int charID;
	char* typeChar;
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
sem_t threeBluesMax;
sem_t threeRedsMax;
visitData* ourVisitData;





/*for either ninja or pirate, the structure goes as follows:
	- try to be one of the first three ninjas or pirates in the ready state (3 ninjas and 3 pirates can hold threeBluesMax and threeRedsMax)
	- if they are one of the 3 at the front of the line, they try to enter the shop
		- they cannot enter the shop if the other team is in the shop (holds their team's lock)
	- if they get into the shop, they get their costume built
	- when the last member exits and the shop will be empty, each team checks if the other team is waiting, if so it gives the thread to the other team and lets three go
*/
void* thread (void* arg)
{ 
	//acquire all our variables
	threadArguments thisArg = *((threadArguments*) arg);
	int charID = thisArg.charID;
	char* typeChar = thisArg.typeChar;
	int visitNum = thisArg.visitNum;
	//we passed in all our fields
	//TODO @MYO
	//typeChar is just N or P, we use that already
	//charID is what they'll be stored into our visitData struct as, that's why we have it
	//visitNum is always 1 to start
	//visitNum will be visitNum++ when the 25% chance to re-visit goes again
	//at which point, you call thread again
	
	
	
	
	
	
	//if Ninja
	if (typeChar == "N") {//if blue

		//try to be one of the three blues that exist
		sem_wait(&threeBluesMax);
		
		//check if the 3 blue turns are taken
		//if so, sleep until a red wakes you up specifically
		if (turnsRemainingBlue <= 0) {
			waitingBlues++;
			sem_wait(&bluesTurn);
		}
		
		//enter
		printf("blue TRYINg to enter\n");
		sem_wait(&mutex_blue);
			printf("blue lock acquired = blue 1\n");
			bluec++;
			turnsRemainingBlue--;
			if (bluec == 1) {
				sem_wait(&mutex_red); //lock out all reds
				printf("red lock acquired = blue 1\n");
			}
		sem_post(&mutex_blue);
		printf("blue lock released\n");
		
		//create the costume
		printf("WE ENTER BLUE\n");
		sleep(1);
		printf("WE EXIT-BLUE\n");
		
		//LEAVE
		sem_wait(&mutex_blue);
			printf("blue lock acquired = blue 2\n");
			bluec--;
			if (bluec == 0) {//if the last blue is exiting
				//THIS SECTION HANDLES THE NEXT TURN; 
				//IF there is a red waiting, let red go
				//if there is no red let blue go
				if (waitingReds > 0) {	
					turnsRemainingRed = 3; //set the next group of turns to 3
					for (int i = 0; i < 3; i++) {
						if (waitingReds > 0) {
							printf("trying to wake up a red\n");
							sem_post(&redsTurn);
							waitingReds--;
						}
					}
				}
				else {
					turnsRemainingBlue = 3; //set the next group of turns to 3
					for (int i = 0; i < 3; i++) {
						if (waitingBlues > 0) {
							printf("trying to wake up a blue\n");
							sem_post(&bluesTurn);
							waitingBlues--;
						}
					}
				}
				sem_post(&mutex_red);
				printf("red lock released\n");
			}
		sem_post(&mutex_blue);
		printf("blue lock released\n");
		
		
		sem_post(&threeBluesMax);
		
		
	}
	
	
	//if Pirate
	else if (typeChar == "P") { //if red
	
		//try to be one of the only 3 reds
		sem_wait(&threeRedsMax);
	
		//check if the 3 blue turns are taken
		//if so, sleep until a red turn goes again
		if (turnsRemainingRed <= 0) {
			waitingReds++;
			sem_wait(&redsTurn);
		}
		
		//enter
		printf("red TRYINg to enter\n");
		sem_wait(&mutex_red);
			printf("red lock acquired = red 1\n");
			redc++;
			turnsRemainingRed--;
			if (redc == 1) { //lock out all blues
				sem_wait(&mutex_blue);
				printf("blue lock acquired = red 1\n");
			}
		sem_post(&mutex_red);
		printf("red lock released\n");
		
		//costume time
		printf("WE ENTER RED\n");
		sleep(1);
		printf("WE STOP RED\n");
		
		//LEAVE
		sem_wait(&mutex_red);
			printf("red lock acquired = red 2\n");
			redc--;
			if (redc == 0) { //if the last red is out
				//THIS SECTION HANDLES THE NEXT TURN; 
				//IF there is a blue waiting, let red go
				//if there is no blues waiting, let red go
				if (waitingReds > 0) {
					turnsRemainingBlue = 3; //set the next group of turns to 3
					for (int i = 0; i < 3; i++) {
						if (waitingBlues > 0) {
							printf("trying to wake up a blue\n");
							sem_post(&bluesTurn);
							waitingBlues--;
						}
					}
				}
				else {
					turnsRemainingRed = 3; //set the next group of turns to 3
					for (int i = 0; i < 3; i++) {
						if (waitingReds > 0) {
							printf("trying to wake up a red \n");
							sem_post(&redsTurn);
							waitingReds--;
						}
					}
				}
				sem_post(&mutex_blue);
				printf("blue lock released\n");
			}
		sem_post(&mutex_red);
		printf("red lock released\n");
		
		sem_post(&threeRedsMax);

	}
	
	
} 


int main(int argc, char* argv[]) {

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
	sem_init(&threeBluesMax, 0, 3);
	sem_init(&threeRedsMax, 0, 3);
	bluec = 0;
	redc = 0;
	turnsRemainingRed = 3;
	turnsRemainingBlue = 3;
	waitingBlues= 0;
	waitingReds = 0;
	ourVisitData = (visitData*)malloc(100*sizeof(visitData));
	
	
	//TODO @myo put thread creation into a loop please, with random times
	//start up some threads
	
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
    sem_destroy(&numRedsFree);
    sem_destroy(&redsTurn);
    sem_destroy(&bluesTurn);
    sem_destroy(&numBluesFree); 
    return 0; 


	


	
}