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
typedef struct line{
	unsigned long tag; 	   // Store tag bits.
	bool valid;		   // Store valid bit
	unsigned long pageNumber;  // Store the starting address of the block in cache.
	//QNode * queueLocation	   // Point to the Node in queue where this pageNumber located.
}line;

/* LRU implementation */

typedef struct QNode {
	struct QNode * toHeadOfQ; // prev Point to the next node towards the head of the queue.
	struct QNode * toTailOfQ; // next Point to the next node towards the tail of the queue.
	unsigned long pageNumber;
} QNode;

/* A Queue*/
typedef struct Queue {
	unsigned count;
	unsigned numberOfFrames;
	QNode * head; //front
	QNode * tail; //rear
}Queue;

/* A utility function to create a new Queue Node. The queue Node will store
 * the given 'pageNumber' */
QNode * newQNode(unsigned long pageNumber) {
	QNode * temp = (QNode *) malloc(sizeof(QNode));
	temp->pageNumber = pageNumber;
	temp->toHeadOfQ = temp->toTailOfQ = NULL;
	return temp;
}

/* A utility function to create an empty Queue.
 * The queue can have at most 'numberOfFrames' nodes. */
Queue * createQueue(int numberOfFrames) {
	Queue * queue = (Queue *) malloc(sizeof(Queue));
	queue->count = 0; //when do we use this? relevant to me?
	queue->head = queue->tail = NULL;
	queue->numberOfFrames = numberOfFrames; //when do we use this? relevant to me?
	return queue;
}
	

/* A utility function to check if queue is empty */
int isQueueEmpty(Queue * queue) {
	return queue->tail == NULL;
}

/* A utility function to delete a frame from queue */
void deQueue(Queue * queue) {
	if (isQueueEmpty(queue)) {
		return;
	}

	// If this is the only node in the list, then change front.
	if (queue->head == queue->tail) {
		queue->head = NULL;
	}

	// Change tail and remove the previous tail.
	QNode * temp = queue->tail; // Save ptr to current tail node, so you can free it later.
	queue->tail = queue->tail->toHeadOfQ; // Point tail to one node up the queue.

	if (queue->tail) {
		queue->tail->toTailOfQ = NULL; // Update pointer of the tail Node.
	}

	free(temp); // Free up the memory of the dequeued node.
}

/* A function to add a page with given 'pageNumber' to both queue and hash. */
void Enqueue (Queue * queue, unsigned pageNumber) {
	
	// Create a new node with given pageNumber.
	// And add the new node to the front of queue.
	QNode * temp = newQNode(pageNumber);
	temp->toTailOfQ = queue->head; // Point new node to the head node of queue.

	// If queue is empty, point head and tail of queue to new node.
	if (isQueueEmpty(queue)) {
	       queue->head = queue->tail = temp;
	}

	else { // Change the front.
		queue->head->toHeadOfQ = temp; // Node of top of Q moves temp ahead of itself.
		queue->head = temp; // Q points to temp as its head.
	}
}


/* This function is called when a page with given 'pageNumber' is referenced
 * from cache (or memory). There are two cases:
 * 1. Frame is not there in memory, we bring it in memory and add to the front
 * of queue.
 * 2. Frame is there is memory, we move the frame to front of queue.
 *
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
*/

/* Function to find the pageNumber from any given address.
 * Given an address and b [no. of bits for blocksize], we can calculate the base address
 * of any block by zeroing out the lowest b bits of the address.
 */
unsigned long pageNumberFinder(unsigned long address, int b){
	return ((address >> b) << b);
}



int main(int argc, char *argv[]){
	int opt; // Store return value from getopt().
	unsigned long sets; // sets = 2^s, where is take from command line args.
	int lines; // Store E from command line args.
	int b;
	int blockSize; // blocksSize = 2^b, where is taken from command line args. 
	FILE * pFile; //pointer to FILE object
	
	while ((opt = getopt(argc, argv, "s:E:b:t:h")) != -1) {
		switch (opt) {
			case 's':
				sets = powfunc(2, atoi(optarg));
				printf("s: %d, sets: %ld\n", atoi(optarg), sets);
				break;
			case 'E':
				lines = atoi(optarg);
				printf("E: %d\n", atoi(optarg));
				break;
			case 'b':
				b = atoi(optarg);
				blockSize = powfunc(2, atoi(optarg));
				printf("b: %d, blockSize: %d\n", b, blockSize);
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

		//printf("buffer: %s\n", buffer);
		//printf("identifier: %c\n", identifier);
		//printf("address in long decimal: %ld\n", address);		
		//printf("address in hex: %lx\n", address);		
		//printf("size: %d\n", size);		
		//unsigned long pageNumber = pageNumberFinder(address, b);
		//printf("pageNumber: %ld\n\n", pageNumber);
		
	}

	fclose(pFile);
	unsigned long add = 31;
	add = pageNumberFinder(add, b);
	printf("pageNumber: %ld\n\n", add);

	/* Dynamically allocate memory for the cache based on the no. and sets and lines. */
	//line * cache = malloc(sizeof(line) * sets * lines);
	
	//free(cache);
	// printSummary(0, 0, 0);
	return 0;

}
