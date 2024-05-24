/*	
	Author: Kaya Nelson

	Description: This example is meant to display deadlock control through the use of
		a timer, where a process is cancelled when it does not have access
		to a resource after a certain period of time. Afterwords it is restarted and 
		allowed to run to completion.
*/	

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

pthread_t tid[4];
pthread_mutex_t lock;
int counter;
int thread_id [4];
int shared_data = 33;
FILE * file;

//Test method to check process functionality of threads, successful
void* test (){
	pthread_mutex_lock(&lock);
	counter += 1;
	printf("Process %d accessing counter %d \n", getpid(), counter);
	sleep(3);
	printf("Process %d exiting.\n", counter);
	pthread_mutex_unlock(&lock);
	return NULL;
}
//Critical section of process, somewhat seuccessful
void* processFunction (){
	char message [100];
	strcpy(message, "Process %d accessing shared resource, num: %d \n");
	printf(message, getpid(), shared_data);
	sleep((int)0.5);
	fprintf(file, message, getpid(), shared_data);
	pthread_mutex_unlock(&lock);
	return NULL;
}
//Section to test for timeout and call critical section, somewhat successful
void* testLock (){
	clock_t start_clock = clock();
	char message [100];
	counter += 1;
	thread_id[counter] = counter;
	while (!pthread_mutex_trylock(&lock))
	{
		if ((clock() - start_clock) > 2) 
		{
			strcpy(message, "Thread %d is starved. Entering sleep mode.\n");
			printf(message, getpid());
			fprintf(file, message, getpid());
			sleep(2);
			strcpy(message, "Thread %d has woken from its sleep.\n");
			printf(message, getpid());
			fprintf(file, message, getpid());
			start_clock = clock();
		}
	}
	processFunction();
	
	//kill(getpid(), SIGSTOP);
	//kill(getpid(), SIGCONT);
}
//One method to utilize pthreads to handle shared resource
void* test_call_one (){
	//Variables to initialize processes
	int i = 0;
	int y = 0;
	file = fopen("deadlockoutput.txt", "w");
	pthread_mutex_init(&lock, NULL);

	//Create and assign the body of each thread
	while (i < 4) {
		pthread_create(&(tid[i]), NULL, &testLock, NULL);
		i++;
	}
	//Join threads for execution
	while (y < 4) {
		pthread_join(tid[y], NULL);
		y++;
	}
	//Destroy the mutex lock and exit
	pthread_mutex_destroy(&lock);
}
//function to access shared resource
void* thread_function (){
	//clock timer for thread stop
	clock_t start_clock = clock();
	char message [100];
	printf("Counter increment before critical selection : %d\n", ++counter);
	fprintf(file, "Counter increment before critical selection : %d\n", ++counter);
	//loop to check for process starvation
	while (!pthread_mutex_trylock(&lock))
	{
		//conditional to trigger when starved
		if ((clock() - start_clock) > 2) 
		{
			strcpy(message, "Thread %d is starved. Entering sleep mode.\n");
			printf(message, getpid());
			fprintf(file, message, getpid());
			//process downtime before restart
			printf("Counter increment inside critical selection: %d\n", ++counter);
			fprintf(file, "Counter increment inside critical selection: %d\n", ++counter);
			sleep(2);
			strcpy(message, "Thread %d has woken from its sleep.\n");
			printf(message, getpid());
			fprintf(file, message, getpid());
			//reset clock in case of additional starvation
			start_clock = clock();
		}
	}
	strcpy(message, "Process %d accessing shared resource, num: %d \n");
	printf(message, getpid(), shared_data);
	sleep((int)0.5);
	fprintf(file, message, getpid(), shared_data);
	pthread_mutex_unlock(&lock);
	printf("Counter increment after critical selection: %d\n", ++counter);
	fprintf(file, "Counter increment after critical selection: %d\n", counter);
}
//function to create and assign thread function
void* access_resource (){
	pthread_t thread;
	pthread_create(&thread, NULL, thread_function(), NULL);
}
//Method to generate processes with unique PID's
void* test_call_two (){
	file = fopen("deadlockoutput.txt", "w");
	pthread_mutex_init(&lock, NULL);
	fork();
	fork();
	access_resource();
}
//main method to create and join threads, successful
int main (void){
	//test_call_one();
	test_call_two();
	return 0;
}
