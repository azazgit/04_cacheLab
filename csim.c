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
 * Cache will be implemented using sets.
 ** Each set is represented by a queue.
 ** The queues are implemented using doubly linked list of nodes.
 ** Each node represents one line of the set.
 */ 

typedef struct Line {
	unsigned long tag; // Within in each set, the tag uniquely identifies the line.
	struct Line * ahead;// Point to the line ahead of this one  in the queue.
	struct Line * behind;// Point to line behind this one in the queue.
	int valid;// vaild bit.
}Line;

typedef struct Set {
	Line * head;// Points to the most recently used line. Head of queue. 
	Line * tail;// Points to the least recently used line. Tail of queue.
	unsigned occupied;// Number of lines in use.
	unsigned capacity;// Taken from cmd line arg 'E'.
}Set;

 /* Use this function to create a new line[node] to add to the set[queue]. */
Line * newLine(unsigned long tag) {
	Line * temp = (Line *) malloc(sizeof(Line));
	temp->ahead = temp->behind = NULL;
	temp->tag = tag;
	temp->valid = 1;
	return temp;
}

/* Function creates an empty Queue.
 * It holds the no. of lines in each set so that it can determine when to 
 * to apply LRU replacement policy. */
set * newSet(int capacity) {
	Set * newSet = (Set *) malloc(sizeof(Set));
	newSet->head = newSet->tail = NULL;
	newSet->occupied = 0; 
	newSet->capacity = capacity;
	return newSet;
}
	
/* Function checks if set is empty. Returns 1 if empty, else 0.*/
int isSetEmpty(Set * set) {
	return set->tail == NULL;
}

/* Function checks if set is full. Returns 1 if full, else 0*/
int isSetFull(Set * set){
	return set->capacity == set->occupied;
}


/* Function removes a line from set [dequeues node of queue].*/
void removeLine(Set * set) {
	if (isSetEmpty(set)) {
		return;
	}

	// If this is the only line in the set, then set becomes empty.
	if (set->head == set->tail) {
		set->head = NULL;
	}

	// Change tail and remove the previous tail.
	Line * temp = set->tail; // Ptr to the line which is to be removed.
	set->tail = set->tail->ahead; // Reassign LRU line of set.

	if (set->tail) {
		set->tail->behind = NULL; // Update set so its new tail points to NULL.
	}

	free(temp); // Free up the memory of the removed line [dequeued node].
}

/* Function creates a new line and adds it to set [enqueues new node to queue].
 * Use when there is a cache miss, i.e., when line is not already in the set.*/
void addLine(Set * set, unsigned long tag) {
	
	Line * temp = newLine(tag);// Create a new line [new node].
	
	// Add new line to set [node goes at the head of queue].
	temp->behind = set->head;

	// Both head and tail of an empty queue must point to the new node.
	if (isSetEmpty(set)) {
	       set->head = set->tail = temp;
	}

	else {// Head of queue points to the new node.
		queue->head->ahead = temp; // Node at top of Q moves temp ahead of itself.
		queue->head = temp; // Q points to temp as its head.
	}
}

/* Function moves existing node to the head of the queue.
 * Use when there is a cache hit, i.e., for when line is already in set.*/
void MoveToHeadOfQ(Set * set, Line * line) {
	
	// When node[line] is not at the head of queue[set].
	if (line != set->head) {
		// Unlink node from its current location in queue.
		line->ahead->behind = line->behind;

		if (line->behind) {// NULL when node is at tail of Q.
			line->behind->ahead = line->ahead;
		}

		// If at tail of Q, then change tail as this node with be moved to front.
		if (line == set->tail) {
			set->tail = line->ahead;
			set->tail->behind = NULL;
		}

		// Move node to head of Q.
		line->behind = set->head;
		line->ahead = NULL;

		// Point prev head of Q to the new one.
		line->behind->ahead = line;

		// Update queue to point to the new head.
		set->head = line;
	}
}


/* Function returns the set index when given an address */
//https://courses.cs.washington.edu/courses/cse378/09wi/lectures/lec15.pdf
// https://courses.cs.washington.edu/courses/cse378/09wi/lectures/lec16.pdf
unsigned long getSetIndex(unsigned long address, unsigned long sets, int blockSize){
	return (address/sets)%blockSize; 
}

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
		
