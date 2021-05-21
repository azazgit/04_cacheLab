#include "cachelab.h"
#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#define EXIT_FAILURE 1
void print_usage(){
	printf("Usage: ./csim -s <s> -E <E> -b <b> -t <tracefile>\n");
}

/* Function prints help instructions for when -h cmd line arg is used. */
void print_help() {
    printf("-h: Optional help flag that prints usage info\n\n");
    printf("-v: Optional verbose flag that displays trace info\n\n");
    printf("-s <s>: Number of set index bits (S = 2^s is the number of sets)\n\n");
    printf("-E <E>: Associativity (number of lines per set)\n\n");
    printf("-b <b>: Number of block bits (B = 2b is the block size)\n\n");
    printf("-t <tracefile>: Name of the valgrind trace to replay\n\n");
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
 * Cache is implemented as an array of set pointers.
 * Each set pointer points to a set.
 * Each set is a Set struct [see below].
 * Each set keeps track of its line as a queue, pointing to the head and tail of its queue.
 * The queues are implemented using doubly linked list of nodes.
 * Each node represents one line of the set.
 */ 

typedef struct Line {
	unsigned long tag; // Within in each set, the tag uniquely identifies the line.
	struct Line * ahead;// Point to the line ahead of this one in the queue.
	struct Line * behind;// Point to line behind this one in the queue.
	int valid;// vaild bit.
}Line;

typedef struct Set {
	Line * head;// Points to the most recently used line. Head of queue. 
	Line * tail;// Points to the least recently used line. Tail of queue.
	unsigned occupied;// Number of lines in use.
	unsigned capacity;// Taken from cmd line arg 'E'.
}Set;

/* Function checks if set is empty. Returns 1 if empty, else 0.*/
int isSetEmpty(Set * set) {
    assert(set != NULL);
    return set->tail == NULL;
}

/* Function checks if set is full. Returns 1 if full, else 0*/
int isSetFull(Set * set){
	assert(set != NULL);
    return set->capacity == set->occupied;
}

/* Use this function to create a new line[node] to add to the set[queue]. */
Line * newLine(unsigned long tag) {
	Line * temp = (Line *) malloc(sizeof(Line));
	temp->ahead = temp->behind = NULL;
	temp->tag = tag;
	temp->valid = 1;
	return temp;
}

/* Function adds newly created line to the head of set [enqueues new node to queue].
 *
 * Use when there is a cache miss, i.e., when line is not already in the set.*/
void addLine(Set * set, unsigned long tag) {
    
    assert(set != NULL);

	Line * temp = newLine(tag);// Create a new line [new node].
	
	// Add new line to set [node goes at the head of queue].
	temp->behind = set->head;

	// Both head and tail of an empty queue must point to the new node.
	if (isSetEmpty(set)) {
	       set->head = set->tail = temp;
	}

	else {// Head of queue points to the new node.
		set->head->ahead = temp; // Node at top of Q moves temp ahead of itself.
		set->head = temp; // Q points to temp as its head.
	}
    set->occupied++;
}

/* Function moves existing node to the head of the queue.
 * Use when there is a cache hit, i.e., for when line is already in set.*/
void moveToHeadOfQ(Set * set, Line * line) {
    
    assert(set != NULL && line != NULL);    
	
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

/* Function removes a line from set [dequeues node of queue].*/
void removeLine(Set * set) {
	
    assert(set != NULL);

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

    set->occupied--;

	free(temp); // Free up the memory of the removed line [dequeued node].
}

/* Function returns the set index when given an address */
//https://courses.cs.washington.edu/courses/cse378/09wi/lectures/lec15.pdf [/lec16.pdf]
unsigned long getSetIndex(unsigned long address, unsigned long sets, int blockSize){
	return (address/blockSize)%sets; 
}

/* Function returns the tag when given an address, s and b bits. */
unsigned long getTag(unsigned long address, int s, int b) {
	return address >> (s + b);
}

/* Function checks if a line is in set. Use to see  if address exists in cache.
 * Returns pointer to the line in set.*/
Line * findLineInSet(Set * set, unsigned long tag){
    
    assert(set != NULL);
    
    Line * temp = set->head; // Point to the most recently used line in set.
    
    while(temp){// While there is a line to check...
        if(temp->valid && temp->tag == tag){// check if line exists
            return temp;
        }
        temp = temp->behind;// else check the next line in the set.
    }
    return NULL;
}

void printSummary(int hit, int miss, int eviction){
    printf("hits:%d misses:%d evictions:%d\n", hit, miss, eviction);
}

int main(int argc, char *argv[]){
	
	/* Parse command line args. */
	int opt; 
	int s;
	int lines; // E 
	int b;
    int verbose = 0;
    FILE * pFile; 
	while ((opt = getopt(argc, argv, "s:E:b:t:hv")) != -1) {
		switch (opt) {
			case 's':
				s = atoi(optarg);
				break;
			case 'E':
				lines = atoi(optarg);
				break;
			case 'b':
				b = atoi(optarg);
				break;
			case 't':
				pFile = fopen(optarg, "r");
				if (pFile == NULL) {
					printf("Unable to open %s\n", optarg);
					print_usage();
	                exit(EXIT_FAILURE);
				}		
				break;
			case 'h':
				print_usage();
                print_help();
                break;
            case 'v':
                printf("verbose chosen. \n");
                verbose = 1;
                break;
			default: /* '?' */
				print_usage();
                exit(EXIT_FAILURE);
		}
	}
	/* End of Parse command line args. */
	
	/* Set up cache data structure. */
	unsigned long sets = powfunc(2, s);
	printf("sets: %ld\n", sets);
	int blockSize = powfunc(2, b);
	printf("blockSize: %d\n", blockSize);
    printf("================\n\n");	


	// Cache holds ptr to array of set ptrs.
	Set ** cache = (Set **) malloc(sizeof(Set *) * sets);
	
    // Each set ptr points to a Set struct.
    int i;
    for(i = 0; i < sets; i++){
        cache[i] = malloc(sizeof(Set));
        cache[i]->head = NULL;
        cache[i]->tail = NULL;
        cache[i]->occupied = 0;
        cache[i]->capacity = lines;
	}
	/* End of Set up cache data structure. */

	/* Parse trace file */ 
	char identifier;
	unsigned long address;
	int size;
	char buffer[50]; // fgets() uses buffer to store each new line from tracefile.
	int hit = 0;
	int miss = 0;
    int evictions = 0;
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
		printf("address: %ld\n", address);		
		//printf("size: %d\n", size);		
		
        // Get set index and tag for the given address.
		unsigned long setIndex = getSetIndex(address, sets, blockSize);
		printf("set index: %ld\n", setIndex);		
		unsigned tag = getTag(address, s, b); 
		printf("tag: %d\n\n", tag);		
		
		// If Load instruction:
		if (identifier == 'L' || identifier == 'S') {
            
            // If set is empty, add the address to cache.
            if(isSetEmpty(cache[setIndex])){
                addLine(cache[setIndex], tag);
                miss++;
		        printf("Empty cache miss. \n");
                printf("buffer: %s ", buffer);
                printf("miss\n");
                printf("==================\n\n");
            }

            // Set is not empty.
            else {// Check if address exists in cache.

                Line * lineFound = findLineInSet(cache[setIndex], tag);
                
                if(lineFound) {// Address is in cache.   
                    hit++;
		            printf("buffer: %s ", buffer);
                    printf("hit\n");
                    printf("==================\n\n");
                    moveToHeadOfQ(cache[setIndex], lineFound);//Update queue.
                }

                else{// Address is not in cache.
                    miss++;
                    printf("Address not in cache miss.\n");
		            printf("buffer: %s ", buffer);
                    printf("miss\n");
                    printf("==================\n\n");

                    printf("runs to here.");
             //       moveToHeadOfQ(cache[setIndex], lineFound);//Update queue.
             //       printf("after movetoHeadOfQ function\n.");
                    if(isSetFull(cache[setIndex])){
                        removeLine(cache[setIndex]);
                        evictions++;
                        printf("address is not in cache. L eviction\n");
                    }
                    addLine(cache[setIndex], tag);
                }
            }
	    }
		//end of if (identifier == 'L') {
        
        // If store instruction:
        else if(identifier == 'M') {
            
            // If set is empty, add the address to cache.
            if(isSetEmpty(cache[setIndex])){
                addLine(cache[setIndex], tag);
                miss++; // load is a miss.
		        printf("Empty cache miss. \n");
                printf("buffer: %s ", buffer);
                printf("miss\n");
                printf("==================\n\n");
                hit++; // store is a hit.
		        printf("Hit after empty cache miss. \n");
                printf("buffer: %s ", buffer);
                printf("hit\n");
                printf("==================\n\n");
            }

            // Set is not empty.
            else{
                Line * lineFound = findLineInSet(cache[setIndex], tag);

                if(lineFound) {// Addres is in cache.
                    hit++; // load is a hit.
		            printf("buffer: %s ", buffer);
                    printf("load hit\n");
                    printf("==================\n\n");
                    hit++; // store is a hit.
		            printf("buffer: %s ", buffer);
                    printf("store hit\n");
                    printf("==================\n\n");
                    moveToHeadOfQ(cache[setIndex], lineFound);
                }

                else {// Address is not in cache.
                    miss++; // load is a miss.
		            printf("buffer: %s ", buffer);
                    printf("load miss\n");
                    printf("==================\n\n");
                    hit++; // store is a hit.
		            printf("buffer: %s ", buffer);
                    printf("store hits after load miss\n");
                    printf("==================\n\n");
                    if(isSetFull(cache[setIndex])){
                        removeLine(cache[setIndex]);
                        evictions++;
                        printf("eviction\n");
                    }
                    addLine(cache[setIndex], tag);
                }
            }
        }
    }
	fclose(pFile);
	/* End of Parse trace file. */

    for (i = 0; i < sets; i++) {
	    while(!isSetEmpty(cache[i])){
           removeLine(cache[i]); // Dequeue each node in queue.
        }
        free(cache[i]);
        cache[i] = NULL;
    } 
    free(cache);
	cache = NULL;
	/* End of free all dynamically allocated memory. */

    printSummary(hit, miss, evictions);

    return 0;
}
