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
    printf("-h: Optional help flag that prints usage info\n\n"
            "-v: Optional verbose flag that displays trace info\n\n"
            "-s <s>: Number of set index bits (S = 2^s is the number of sets)\n\n"
            "-E <E>: Associativity (number of lines per set)\n\n"
            "-b <b>: Number of block bits (B = 2b is the block size)\n\n"
            "-t <tracefile>: Name of the valgrind trace to replay\n\n");
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

void printSummary(int hits, int misses, int evictions){
    printf("hits:%d misses:%d evictions:%d\n", hits, misses, evictions);
}


Set ** setUp_cache(int b, int s, int E) {
	
    // Get no of sets and block size.
    unsigned long sets = 1 << s;
    int blockSize = 1 << b;

    // Cache holds ptr to array of set ptrs.
	Set ** cache = (Set **) malloc(sizeof(Set *) * sets);
	
    // Each set ptr points to a Set struct.
    int i;
    for(i = 0; i < sets; i++){
        cache[i] = malloc(sizeof(Set));
        cache[i]->head = NULL;
        cache[i]->tail = NULL;
        cache[i]->occupied = 0;
        cache[i]->capacity = E;
    }
    return cache;
}

int main(int argc, char *argv[]){
	
	/* Parse command line args. */
	int opt; 
	int s;
	int E; // lines 
	int b;
    int verbose = 0;
    FILE * pFile; 
	while ((opt = getopt(argc, argv, "s:E:b:t:hv")) != -1) {
		switch (opt) {
			case 's':
				s = atoi(optarg);
				break;
			case 'E':
				E = atoi(optarg);
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
                verbose = 1;
                break;
			default: /* '?' */
				print_usage();
                exit(EXIT_FAILURE);
		}
	}
	/* End of Parse command line args. */
	
	/* Set up cache data structure. */
    Set ** cache = setUp_cache(b, s, E); 
	
    /* Parse trace file */ 
    unsigned long sets = 1<<s;
	char identifier;
	unsigned long address;
	int size;
	char buffer[50]; // fgets() uses buffer to store each new line from tracefile.
	int hits= 0;
	int misses = 0;
    int evictions = 0;
	while(fgets(buffer, 50, pFile) > 0) {
		
        buffer[strlen(buffer)-1] = '\0';// Remove trailing '\n' in the line in file.
		
		if (buffer[0] == ' '){// Line has M, L or S.
			sscanf(buffer, " %c %lx,%d\n", &identifier, &address, &size);
		}
        else {continue;} // Ignore I instructions and go to next line in file.
		
        // Get set index and tag for the given address.
        int blockSize = 1<<b;
		unsigned long setIndex = (address / blockSize) % sets;
		unsigned tag = address >> (s + b); 
		
        // When set is empty...
        if(isSetEmpty(cache[setIndex])){
            
            addLine(cache[setIndex], tag);
            misses++; // Load and store will cold miss.
            if (identifier == 'M') {hits++;} // After load, store will hit.    
            
            if (verbose) {
                printf("%s miss", buffer); // For load and store misses.    
                if (identifier == 'M') {printf(" hit");}
                printf("\n");
            }
        }

        // When set is not empty...
        else {
            // Check if address exists in cache.
            Line * lineFound = findLineInSet(cache[setIndex], tag);
            
            if(lineFound) {// Address is in cache.   
                    
                hits++; // Both load and store will hit.
                
                if (identifier == 'M') {hits++;} // After load, store will hit.
		        
                if (verbose) {
                    printf("%s hit", buffer); // For load and store misses.    
                    if (identifier == 'M') {printf(" hit");}
                    printf("\n");
                }

                moveToHeadOfQ(cache[setIndex], lineFound);//Update queue.
            }
            
            else{// Address is not in cache.
                    
                misses++; // Both load and store will miss.
                if (identifier == 'M'){hits++;} // After load, store will hit.    
                
                if(isSetFull(cache[setIndex])){
                    removeLine(cache[setIndex]);
                    evictions++;
                    if (verbose) {
                        printf("%s miss eviction", buffer); // For load and store misses.    
                        if (identifier == 'M') {printf(" hit");}
                        printf("\n");
                    }
                }
                else{// Set is partially full and there is cache miss.
                    if (verbose) {
                        printf("%s miss", buffer); // For load and store misses.    
                        if (identifier == 'M') {printf(" hit");}
                        printf("\n");
                    }
                }
            addLine(cache[setIndex], tag);
            }
        }
    }
	fclose(pFile);
	/* End of Parse trace file. */

    int i;
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

    printSummary(hits, misses, evictions);

    return 0;
}
