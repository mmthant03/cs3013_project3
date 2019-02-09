#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

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





void* thread(void* arg) 
{ 
	char* input = (char*) arg;


	
	
	
	//if Ninja
	if (input == "N") {//if blue
	
		//try to be one of the three blues that exist
		sem_wait(&threeBluesMax);
	
	
		//check if the 3 blue turns are taken
		//if so, sleep until a red wakes you up specifically
		if (turnsRemainingBlue <= 0) {
			waitingBlues++;
			sem_wait(&bluesTurn);
		}
		
		//wait for our mutual exclusion
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
		
		
		
		//critical section
		printf("WE ENTER BLUE\n");
		sleep(1);
		printf("WE EXIT-BLUE\n");
		
		//leave
		sem_wait(&mutex_blue);
			printf("blue lock acquired = blue 2\n");
			bluec--;
			if (bluec == 0) {
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
				
				
				
				
				turnsRemainingRed = 3; //set the next group of turns to 3
				for (int i = 0; i < 3; i++) {
					if (waitingReds > 0) {
						printf("trying to wake up a red\n");
						sem_post(&redsTurn);
						waitingReds--;
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
	else if (input == "P") { //if red
		sleep(0.5);
		sem_wait(&threeRedsMax);
	
		//check if the 3 blue turns are taken
		//if so, sleep until a red wakes you up specifically
		if (turnsRemainingRed <= 0) {
			waitingReds++;
			sem_wait(&redsTurn);
		}
		
		
		printf("red TRYINg to enter\n");
		sem_wait(&mutex_red);
			printf("red lock acquired = red 1\n");

			redc++;
			turnsRemainingRed--;
			if (redc == 1) { //if this is first red
				sem_wait(&mutex_blue);
				printf("blue lock acquired = red 1\n");
			}
		sem_post(&mutex_red);
		printf("red lock released\n");
		
		
		
		//critical section
		printf("WE ENTER RED\n");
		sleep(1);
		printf("WE STOP RED\n");
		
		
		
		
		sem_wait(&mutex_red);
			printf("red lock acquired = red 2\n");
			redc--;
			if (redc == 0) { //last red is out
			
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
	
	
	//start up some threads
    pthread_t t1,t2, t3, t4, t5, t6, t7, t8, t9; 
    pthread_create(&t1,NULL,thread, "N"); 
    pthread_create(&t2,NULL,thread, "N"); 
    pthread_create(&t3,NULL,thread, "N"); 
    pthread_create(&t4,NULL,thread,"N"); 
    pthread_create(&t5,NULL,thread,"N"); 
    pthread_create(&t8,NULL,thread,"N"); 
    pthread_create(&t9,NULL,thread,"P"); 
    pthread_create(&t6,NULL,thread,"N"); 
    pthread_create(&t7,NULL,thread,"N"); 
    sleep(4); 
    pthread_join(t1,NULL); 
    pthread_join(t2,NULL); 
    pthread_join(t3,NULL); 
    pthread_join(t4,NULL); 
    pthread_join(t5,NULL); 
    pthread_join(t6,NULL); 
    pthread_join(t7,NULL); 
    sem_destroy(&numRedsFree);
    sem_destroy(&redsTurn);
    sem_destroy(&bluesTurn);
    sem_destroy(&numBluesFree); 
    return 0; 


	


	
}