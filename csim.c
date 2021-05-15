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
	unsigned tag; 	// Store tag bits
	bool valid;	// Store valid bit
		
	int lru;	// Store Least Recently Used status
}line;



int main(int argc, char *argv[]){
	int opt;
	int sets;
	int lines;
	int blockSize;
	FILE * pFile; //pointer to FILE object
	
	while ((opt = getopt(argc, argv, "s:E:b:t:h")) != -1) {
		switch (opt) {
			case 's':
				sets = powfunc(2, atoi(optarg));
				printf("s: %d, sets: %d\n", atoi(optarg), sets);
				break;
			case 'E':
				lines = atoi(optarg); //pow(2, atoi(optarg));
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

	/* Dynamically allocate memory for the cache based on the no. and sets and lines. */
	line * cache = malloc(sizeof(line) * sets * lines);

	 


	// Reading lines like " M 20,1" or "L 19,3"
	char identifier;
	unsigned long address;
	int size;
	char buffer[50]; // Used in fgets() to store a new line from file.
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

    
	
	

	
	// printSummary(0, 0, 0);
    return 0;
}
