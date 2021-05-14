#include "cachelab.h"
#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
// #include <math.h>

void print_usage(){
	printf("Usage: ./csim -s <s> -E <E> -b <b> -t <tracefile>\n");
	exit(1);
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
	unsigned tag;
	int valid;
	int lru;
}line;



int main(int argc, char *argv[]){
	int opt;
	int sets;
	int lines;
	int blocks;
	FILE * pFile; //pointer to FILE object
	
	while ((opt = getopt(argc, argv, "s:E:b:t:")) != -1) {
		switch (opt) {
			case 's':
				sets = powfunc(2, atoi(optarg));
				break;
			case 'E':
				lines = atoi(optarg); //pow(2, atoi(optarg));
				break;
			case 'b':
				blocks = powfunc(2, atoi(optarg));
				break;
			case 't':
				pFile = fopen(optarg, "r");
				if (pFile == NULL) {
					printf("Unable to open %s\n", optarg);
					print_usage();
				}		
				break;
			default: /* '?' */
				print_usage();
		}
       }

	/* Dynamically allocate memory for the cache based on the no. and sets and lines. */
	line * cache = malloc(sizeof(line) * sets * lines);

	 


	// Reading lines like " M 20,1" or "L 19,3"
	char identifier;
	unsigned address;
	int size;

	while(fscanf(pFile, " %c %x, %d", &identifier, &address, &size) > 0) {
		printf("identifier: %c\n", identifier);		
		printf("address: %x\n", address);		
		printf("size: %d\n", size);		
	}

	fclose(pFile);

    
	
	

	
	// printSummary(0, 0, 0);
    return 0;
}
