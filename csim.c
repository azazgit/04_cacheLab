#include "cachelab.h"
#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define EXIT_FAILURE 1
void print_usage(){
	printf("Usage: ./csim -s <s> -E <E> -b <b> -t <tracefile>\n");
	exit(EXIT_FAILURE);
}

/* Power function to calculate no of sets and block_size */
static int powfunc(int base, int exp) {
	int result = 1;
	int exponent;
	for (exponent = exp; exponent > 0; exponent--){
		result *= base;
	}
	return result;
}

/* Create the line struct */
typedef struct{
	unsigned long tag; 	// Store tag bits.
	bool valid;		// Store valid bit
	unsigned long pageNum	// Store the starting address of the block in cache.
}line;

/* LRU implementation */

typedef struct QNode {
	struct QNode * prev;
	struct QNode * next;
	unsigned long pageNumber;
} QNode;

/* A Queue*/
typedef struct Queue {
	unsigned count;
	unsigned numberOfFrames;
	QNode * front;
	QNode * rear;
} Queue;

/* A hash (Collection of pointers to Queue Nodes). */
typedef struct Hash {
	int capacity;
	QNode ** array;
}

/* A utility function to create a new Queue Node. The queue Node will store
 * the given 'pageNumber' */
QNode * newQNode(unsigned pageNumber) {
	QNode * temp = (QNode *) malloc(sizeof(QNode));
	temp->pageNumber = pageNumber;
	temp->prev = temp->next = NULL;
	return temp;
}

/* A utility function to create an empty Queue.
 * The queue can have at most 'numberOfFrames' nodes. */
Queue * createQueue(int numberOfFrames) {
	Queue * queue = (Queue *) malloc(sizeof(Queue);
	queue->count = 0;
	queue->front = queue->rear = NULL;
	queue->numberOfFrames = numberOfFrames;
	return queue;
}
	
/* A utility function to check if there is slot available in memory. */
int AreAllFramesFull(Queue * queue) {
	return queue->count == queue->numberOfFrames;
}

/* A utility function to check if queue is empty */

int isQueueEmpty(Queue * queue) {
	return queue->rear == NULL;
}

/* A utility function to delete a frame from queue */
void deQueue(Queue * queue) {
	if (isQueueEmpty(queue)) {
		return;
	}

	// If this is the only node in the list, then change front.
	if (queue->front == queue->rear) {
		queue->front = NULL;
	}

	// Change rear and remove the previous rear.
	QNode * temp = queue->rear;
	queue->rear = queue->rear->prev;

	if (queue->rear) {
		queue->rear->next = NULL;
	}

	free(temp);

	// Decrement the number of full frames by 1.
	queue->count--;
}

/* A function to add a page with given 'pageNumber' to both queue and hash. */
void Enqueue (Queue * queue, Hash * hash, unsigned pageNumber) {
	
	// If all frames are full, remove the page at the rear.
	if (AreAllFramesFull(queue)) {
		hash->array[queue->rear->pageNumber] = NULL;
		deQueue(queue);
	}

	// Create a new node with given page number.
	// And add the new node to the front of queue.
	QNode * temp = newQNode(pageNumber);
	temp->next = queue->front;

	// If queue is empty, change both front and rear pointers.
	if (isQueueEmpty(queue)) {
	       queue->rear = queue->front = temp;
	}

	else { // Change the front.
		queue->front->prev = temp;
		queue->front = temp;
	}

	// Add page entry to hash also.
	hash->array[pageNUmber] = temp;

	// Increment number of full frames.
	queue->count++;
}

/* This function is called when a page with given 'pageNumber' is referenced
 * from cache (or memory). There are two cases:
 * 1. Frame is not there in memory, we bring it in memory and add to the front
 * of queue.
 * 2. Frame is there is memory, we move the frame to front of queue.
 */
void ReferencePage(Queue * queue, Hash * hash, unsigned pageNumber) {
	QNode * reqPage = hash->array[pageNumber];

	// The page is not in cache, bring it.
	if (reqPage == NULL) {
		Enqueue(queue, hash, pageNumber);
	}
	else if (reqPage != queue->front) {
		// Unlink requested page from its current location in queue.
		reqPage->prev->next = reqPage->next;
		if (reqPage->next) {
			reqPage->next->prev = reqPage->prev;
		}

		// If the requested page is rear, then change rear as this
		// node with be moved to front.
		if (reqPage == queue->rear) {
			queue->rear = reqPage->prev;
			queue->rear->next = NULL;
		}

		// Put requested page before current front.
		reqPage->next = queue->front;
		reqPage->prev = NULL;

		// Change prev of current front.
		reqPage->next->prev = reqPage;

		// Change front to the requested page.
		queue->front = reqPage;
	}
}



int main(int argc, char *argv[]){
	int opt; // Store return value from getopt().
	unsigned long sets; // sets = 2^s, where is take from command line args.
	int lines; // Store E from command line args.
	int blockSize; // blocksSize = 2^b, where is taken from command line args. 
	FILE * pFile; //pointer to FILE object
	
	while ((opt = getopt(argc, argv, "s:E:b:t:h")) != -1) {
		switch (opt) {
			case 's':
				sets = powfunc(2, atoi(optarg));
				printf("s: %d, sets: %d\n", atoi(optarg), sets);
				break;
			case 'E':
				lines = atoi(optarg);
				printf("E: %d\n", atoi(optarg));
				break;
			case 'b':
				blockSize = powfunc(2, atoi(optarg));
				printf("b: %d, blockSize: %d\n", atoi(optarg), blockSize);
				break;
			case 't':
				pFile = fopen(optarg, "r");
				if (pFile == NULL) {
					printf("Unable to open %s\n", optarg);
					print_usage();
				}		
				break;
			case 'h':
				printf("help option chosen \n");
				printf("Usage: ./csim -s <s> -E <E> -b <b> -t <tracefile>\n");
				break;
			default: /* '?' */
				print_usage();
		}
       }

	// Read lines like " M 20,1" from tracefile, which is  pointed to by pFile.
	char identifier;
	unsigned long address;
	int size;
	char buffer[50]; // Used in fgets() to store a new line from tracefile.
	while(fgets(buffer, 50, pFile) > 0) {
		buffer[strlen(buffer)-1] = '\0'; //remove trailing '\n' in the line in file.
	
		if (buffer[0] == ' '){ // Line has M, L or S.
			sscanf(buffer, " %c %lx,%d\n", &identifier, &address, &size);
		}
		else { // Line has I.
			sscanf(buffer, "%c %lx,%d\n", &identifier, &address, &size); 
		}

		printf("buffer: %s\n", buffer);
		printf("identifier: %c\n", identifier);
		printf("address in long decimal: %ld\n", address);		
		printf("address in hex: %lx\n", address);		
		printf("size: %d\n", size);		
	}

	fclose(pFile);

	/* Dynamically allocate memory for the cache based on the no. and sets and lines. */
	line * cache = malloc(sizeof(line) * sets * lines);

	 


	

	
	// printSummary(0, 0, 0);
    return 0;
}
