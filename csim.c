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


/* LRU implementation
 * Each set in the cache will have its own queue.
 * Each line in a set will have its own node in the queue.
 */ 

typedef struct line { 
	unsigned long tag; // Within in each set, the tag uniquely identifies the line.
	struct line * ahead;// Point to the line ahead of this one  in the queue.
	struct QNode * behind;// Point to line behind this one in the queue.
	int valid;// vaild bit.
}line;

/* A Queue*/
typedef struct Queue {
	unsigned count;
	unsigned linesInSet;
	QNode * head; 
	QNode * tail; 
}Queue;

/* Function creates a new queue node wherein tag is stored. */
QNode * newQNode(unsigned long tag) {
	QNode * temp = (QNode *) malloc(sizeof(QNode));
	temp->tag = tag;
	temp->ahead = temp->behind = NULL;
	return temp;
}

/* Function creates an empty Queue.
 * It holds the no. of lines in each set so that it can determine when to 
 * to apply LRU replacement policy. */
Queue * createQueue(int linesInSet) {
	Queue * queue = (Queue *) malloc(sizeof(Queue));
	queue->count = 0; // Number of lines in use.
	queue->head = queue->tail = NULL;
	queue->linesInSet = linesInSet;
	return queue;
}
	

/* Function checks if queue is empty. */
int isQueueEmpty(Queue * queue) {
	return queue->tail == NULL;
}

/* Function deletes a node from queue. */
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
	queue->tail = queue->tail->ahead; // Point tail to one node up the queue.

	if (queue->tail) {
		queue->tail->behind = NULL; // Update pointer of the tail Node.
	}

	free(temp); // Free up the memory of the dequeued node.
}

/* Function adds node with given 'tag' to both queue and . */
void Enqueue (Queue * queue, unsigned pageNumber) {
	
	// Create a new node with given pageNumber.
	// And add the new node to the front of queue.
	QNode * temp = newQNode(pageNumber);
	temp->behind = queue->head; // Point new node to the head node of queue.

	// If queue is empty, point head and tail of queue to new node.
	if (isQueueEmpty(queue)) {
	       queue->head = queue->tail = temp;
	}

	else { // Change the front.
		queue->head->ahead = temp; // Node of top of Q moves temp ahead of itself.
		queue->head = temp; // Q points to temp as its head.
	}
}


/* Function moves the line referenced by queueLocation to the front of the queue. */
void MoveToFrontOfQ(Queue * queue, QNode * node) {
	
	// When line is not at the front of queue.
	if (node != queue->head) {
		// Unlink the line from its current location in queue.
		node->ahead->behind = node->behind;
		if (node->behind) {// NULL when node is at tail of Q.
			node->behind->ahead = node->ahead;
		}

		// If at tail of Q, then change tail as this node with be moved to front.
		if (node == queue->tail) {
			queue->tail = node->ahead;
			queue->tail->behind = NULL;
		}

		// Move node to head of Q.
		node->behind = queue->head;
		node->ahead = NULL;

		// Point prev head of Q to the new one.
		node->behind->ahead = node;

		// Update queue to point to the new head.
		queue->head = node;
	}
}


/* Create the line struct */
typedef struct line{
	unsigned long tag; 	   // Store tag bits.
	int valid;		   // Store valid bit
	unsigned long pageNumber;  // Store the starting address of the block in cache.
	QNode * queueLocation;	   // Point to the Node in queue where this pageNumber located.
}line;

/* Function to find the page from any given address.
 * Given an address and b [no. of bits for blocksize], we can calculate the base address
 * of any block by zeroing out the lowest b bits of the address.
 * BlockSize determines how many bytes each of line cache contains. Since the project requirement does not care for individual bytes, we only store the address of byte 0 of the block. If any byte in that block were to exist in cache then so would byte 0, since either the whole block exists in cache or none of its bytes. In this way, we only have to store address of one byte.
 */
unsigned long pageFinder(unsigned long address, int b){
	return ((address >> b) << b);
}

/* Function returns the set index when given an address */
unsigned long getSetIndex(unsigned long address, unsigned long sets, int blockSize){
	return (address/sets)%blockSize; //https://courses.cs.washington.edu/courses/cse378/09wi/lectures/lec15.pdf
}// https://courses.cs.washington.edu/courses/cse378/09wi/lectures/lec16.pdf

/* Function returns the tag when given an address, s and b bits. */
unsigned long getTag(unsigned long address, int s, int b) {
	return address >> (s + b);
}





int main(int argc, char *argv[]){
	
	/* Parse command line args. */
	int opt; 
	int s;
	int lines; // E 
	int b;
	FILE * pFile; 
	while ((opt = getopt(argc, argv, "s:E:b:t:h")) != -1) {
		switch (opt) {
			case 's':
				s = atoi(optarg);
				printf("s: %d\n", atoi(optarg));
				break;
			case 'E':
				lines = atoi(optarg);
				printf("E: %d\n", atoi(optarg));
				break;
			case 'b':
				b = atoi(optarg);
				printf("b: %d\n", atoi(optarg));
				break;
			case 't':
				pFile = fopen(optarg, "r");
				if (pFile == NULL) {
					printf("Unable to open %s\n", optarg);
					print_usage();
				}		
				printf("file: %s\n\n", optarg);
				break;
			case 'h':
				printf("help option chosen \n");
				printf("Usage: ./csim -s <s> -E <E> -b <b> -t <tracefile>\n");
				break;
			default: /* '?' */
				print_usage();
		}
	}
	/* End of Parse command line args. */
	
	/* Set up cache data structure. */
	unsigned long sets = powfunc(2, s);
	printf("sets: %ld\n", sets);
	int blockSize = powfunc(2, b);
	printf("blockSize: %d\n", blockSize);

	line ** cache = malloc(sizeof(line*) * sets);// Cache holds ptr to array of sets. 
	int i;
	for (i = 0; i < sets; i++){
		cache[i] = malloc(sizeof(line) * lines);// Each set holds ptr to array of lines.
	}

	int j;
	for (i = 0; i < sets; i++) {// Initialise each line with 0 and null ptr.
		for (j = 0; j < lines; j++) {
			(cache[i][j]).tag = 0;
			cache[i][j].valid = 0;
			cache[i][j].pageNumber = 0;
			cache[i][j].queueLocation = NULL;
		}
	}	
	/* End of Set up cache data structure. */

	/* Set up queue */
	 
	
	/* End of set up queue. */	
	
	/* Parse trace file */ 
	char identifier;
	unsigned long address;
	int size;
	char buffer[50]; // fgets() uses buffer to store each new line from tracefile.
	int hit = 0;
	int miss = 0;
	while(fgets(buffer, 50, pFile) > 0) {
		buffer[strlen(buffer)-1] = '\0';// Remove trailing '\n' in the line in file.
		printf("buffer: %s\n", buffer);
		
		if (buffer[0] == ' '){// Line has M, L or S.
			sscanf(buffer, " %c %lx,%d\n", &identifier, &address, &size);
		}
		else {// Line has I.
			sscanf(buffer, "%c %lx,%d\n", &identifier, &address, &size); 
		}
		printf("identifier: %c\n", identifier);
		printf("address in hex: %lx\n", address);		
		printf("size: %d\n\n", size);		
		
		unsigned long setIndex = getSetIndex(address, sets, blockSize);
		unsigned tag = getTag(address, s, b); 

		// If Load instruction:
		if (identifier == 'L') {
			
			int found = 0;
			int i;
			for (i = 0; i < lines; i++) {
				if((cache[setIndex][i].valid == 1) && 
						(cache[setIndex][i].tag == tag)){
					hit++;
					found = 1;
					//move line to the head of q;
				}
			}

			// Miss
			if(!found){
				miss++;
				// Remove LRU line in set.
				// Add this to the head of q
				// Each set needs to have its own q. Damn!



		} //end of if (identifier == 'L') {
	}
	fclose(pFile);
	/* End of Parse trace file. */


	
	


		}
	// printSummary(0, 0, 0);

	/* Free all dynamically allocated memory for lines, sets and cache. */
	for (i = 0; i < sets; i++) {free(cache[i]);}
	free(cache);
	cache = NULL;
	/* End of free all dynamically allocated memory. */
	return 0;
}



		//unsigned long pageNumber = pageNumberFinder(address, b);
		//printf("pageNumber: %ld\n\n", pageNumber);
		
