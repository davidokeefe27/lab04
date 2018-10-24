#include "cachelab.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
/*
 * David O'Keefe -- okeefed@appstate.edu
 * Steve Lewis -- lewissj@appstate.edu
 *
 */
void getBits(int start, int end, unsigned long bits);
int verboseFlag = 0;
int helpFlag = 0;

void getBits(int start, int end, unsigned long bits) {
    int i;
    unsigned long mask = 0UL;
    if(start < 64 && start >= 0 && end < 64 && end > 0) {
        int diff = end - start;
        for (i = 0; i < diff; i++) {
            mask <<= 1;
            mask |= 1;
        }
        mask <<= start;
        bits &= mask; 
    }
    else {
        printf("Error\n");
        printf("Usage: ./csim-ref [-h] [-v] -s <s> -E <E> -b <b> -t <tracefile>");
        exit(1);
    }
}

struct Cache {
    int numSets;
    int numSetBits;
    int linesPerSet;
    int numBlockBits;
    int blockSize;
    int numTagBits;
    unsigned long int **tags;
};

struct Cache createCache(int numSetBits, int linesPerSet, int numBlockBits) {
    int i, j;
    struct Cache c;
    c.numSetBits = numSetBits;
    c.numSets = pow(2, numSetBits);
    c.linesPerSet = linesPerSet;
    c.numBlockBits = numBlockBits;
    c.blockSize = pow(2, numBlockBits);
    c.numTagBits = 63 - numSetBits - numBlockBits;
    c.tags = malloc(c.numSets * c.linesPerSet * sizeof(unsigned long int));
    //initialize tags to -1
    for (i = 0; i < c.numSets; i++) {
        for(j = 0; j < c.linesPerSet; j++) {
            c.tags[i][j] = -1;
        }
    }
    
    return c;
}



int main(int argc, char **argv)
{
    int c;
    int numSetBits, numLinesPerSet, blockOffsetBits;
    while((c = getopt(argc, argv, "h::v::s:E:b:t:")) != -1) {
        switch(c) {
            case 'h': 
            helpFlag = 1;
            break;
            case 'v':
            verboseFlag = 1; 
            break;
            case 's':
            numSetBits = optarg;
            break;
            case 'E':
            numLinesPerSet = optarg;
            break;
            case 'b':
            blockOffsetBits = optarg;
            break;
            case 't':
            //?
            break;
        }
   struct Cache cache = createCache(numSetBits, linesPerSet, numBlockBits);
   /* 
    if (argc < 8) {
        printf("Error\n");
        printf("Usage: ./csim-ref [-h] [-v] -s <s> -E <E> -b <b> -t <tracefile>");
        exit(1);
    }
    else {
        for(count = 1; count < argc; count++) {
            if(argv[count][0].charAt(0) == '-') { 
                flag = argv[count][1];    
            }
        }
    }*/
    printSummary(0, 0, 0);
    return 0;
}
