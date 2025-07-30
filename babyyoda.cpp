/*************************************************************************************
 * babyyoda - used to test your semaphore implementation and can be a starting point for
 *			     your store front implementation
 *
 *************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include "Semaphore.h"

// Semaphores that each thread will have access to as they are global in shared memory
Semaphore *empty = NULL;
Semaphore *full = NULL;

pthread_mutex_t buf_mutex;
pthread_mutex_t exit_mutex;
pthread_cond_t exit_cond;


bool all_produced = false;
bool all_consumed = false;
int *buffer;
int consumed = 0;


/*************************************************************************************
 * producer_routine - this function is called when the producer thread is created.
 *
 *			Params: data - a void pointer that should point to an integer that indicates
 *							   the total number to be produced
 *
 *			Returns: always NULL
 *
 *************************************************************************************/

void *producer_routine(void *data) {

	time_t rand_seed;
	srand((unsigned int) time(&rand_seed));

	// The current serial number (incremented)
	unsigned int serialnum = 1;
	
	// We know the data pointer is an integer that indicates the number to produce
	int left_to_produce = *((int *) data);

	// Loop through the amount we're going to produce and place into the buffer
	while (left_to_produce > 0) {
		printf("Producer wants to put Yoda #%d into buffer...\n", serialnum);

		// Semaphore check to make sure there is an available slot
		empty->wait();

		// Place item on the next shelf slot by first setting the mutex to protect our buffer vars
		pthread_mutex_lock(&buf_mutex);

		buffer[empty->getCount()] = serialnum; // Place the item on the shelf
		int current_yoda = serialnum;
		serialnum++;
		left_to_produce--;

		printf("   Yoda %d put on shelf.\n", current_yoda);

		if (left_to_produce == 0) {
			all_produced = true; // We are done producing, set the flag
		}

		// Semaphore signal that there are items available
		full->signal();
		pthread_mutex_unlock(&buf_mutex);

		// random sleep but he makes them fast so 1/20 of a second
		usleep((useconds_t) (rand() % 200000));
	}

	return NULL;
}


/*************************************************************************************
 * consumer_routine - this function is called when the consumer thread is created.
 *
 *       Params: id - a void pointer that points to an integer that indicates
 *                     the consumer's ID number
 *
 *       Returns: always NULL
 *
 *************************************************************************************/

void *consumer_routine(void *data) {
	int id = *((int *) data);


	while (1) {
		pthread_mutex_lock(&exit_mutex);
		while (!all_consumed && full->getCount() == 0) {
			pthread_cond_wait(&exit_cond, &exit_mutex);
		}
		if (all_produced && full->getCount() == 0) {
			pthread_mutex_unlock(&exit_mutex);
			break;
		}
		pthread_mutex_unlock(&exit_mutex);
				
		printf("Consumer %d wants to buy a Yoda...\n", id);

		// Semaphore to see if there are any items to take
		full->wait();
		// Take an item off the shelf
		pthread_mutex_lock(&buf_mutex);
	
		if (buffer[empty->getCount()] != 0) {
			printf("   Consumer %d bought Yoda #%d.\n", id, buffer[empty->getCount()]);
			buffer[empty->getCount()] = 0;
			consumed++;
		}

		pthread_mutex_unlock(&buf_mutex);
		empty->signal();

		// Consumers wait up to one second
		usleep((useconds_t) (rand() % 1000000));
	}
	printf("Consumer %d goes home.\n", id);

	return NULL;	
}


/*************************************************************************************
 * main - Standard C main function for our storefront. 
 *
 *		Expected params: pctest <num_consumers> <max_items>
 *				max_items - how many items will be produced before the shopkeeper closes
 *
 *************************************************************************************/

int main(int argv, const char *argc[]) {

	// Get our argument parameters
	if (argv < 4) {
		printf("Invalid parameters. Format: %s <buffer_size> <num_consumers> <max_items>\n", argc[0]);
		exit(0);
	}

	// User input on the size of the buffer
	int buffer_size = (unsigned int) strtol(argc[1], NULL, 10);
	unsigned int num_consumers = (unsigned int) strtol(argc[2], NULL, 10);
	int num_produce = (unsigned int) strtol(argc[3], NULL, 10);

	buffer = new int[buffer_size];

	printf("Producing %d today.\n", num_produce);
	
	// Initialize our semaphores
	empty = new Semaphore(buffer_size);
	full = new Semaphore(0);

	pthread_mutex_init(&buf_mutex, NULL); // Initialize our buffer mutex
	pthread_mutex_init(&exit_mutex, NULL); // Initialize mutex for exit condition
	pthread_cond_init(&exit_cond, NULL);   // Initialize condition variable for exit

	pthread_t producer;
	pthread_t* consumers = new pthread_t[num_consumers];

	// Launch our producer thread
	pthread_create(&producer, NULL, producer_routine, (void *) &num_produce);

	// Launch our consumer threads
	for (unsigned long i = 0; i < num_consumers; i++) {
		pthread_create(&consumers[i], NULL, consumer_routine, (void *) &i);
	}
	

	// Wait for our producer thread to finish up
	pthread_join(producer, NULL);

	printf("The manufacturer has completed his work for the day.\n");

	printf("Waiting for consumer to buy up the rest.\n");

	// Give the consumers a second to finish snatching up items
	while (consumed < num_produce) {
		sleep(1);
	}


	pthread_mutex_lock(&exit_mutex);
	all_consumed = true; // Set the flag to indicate all items are consumed
	pthread_cond_broadcast(&exit_cond); // Wake up all waiting consumers
	pthread_mutex_unlock(&exit_mutex);


	// Now make sure they all exited
	for (unsigned int i=0; i<num_consumers; i++) {
		pthread_join(consumers[i], NULL);
	}

	// We are exiting, clean up
	delete empty;
	delete full;
	delete[] consumers;
	delete[] buffer;

	printf("Producer/Consumer simulation complete!\n");

}
