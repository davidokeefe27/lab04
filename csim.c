#include "cachelab.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
/*
 * David O'Keefe -- okeefed@appstate.edu
 * Steve Lewis -- lewissj@appstate.edu
 *
 */
//Function Declarations
int parseCommandLine(int argc, char **argv, int *cache_s, int *cache_E, int *cache_b, char **trace, int *vflag);
uint64_t getBits(int start, int end, unsigned long bits);
void errorMessage();
struct Cache *createCache(struct Cache *c, int numSetBits, int linesPerSet, int numBlockBits);
int parseTraceFile(FILE * pf, char ** trace, struct Cache *cache);

//Global Variable(s)
int verboseFlag = 0;
int helpFlag = 0;

void errorMessage() { 
    printf("Error\n");
    printf("Usage: ./csim-ref [-h] [-v] -s <s> -E <E> -b <b> -t <tracefile>");
}

uint64_t getBits(int start, int end, unsigned long bits) {
    int i;
    unsigned long mask = 0UL;
    if(start < 64 && start >= 0 && end < 64 && end >= 0) {
        int diff = end - start;
        for (i = 0; i < diff; i++) {
            mask <<= 1;
            mask |= 1;
        }
        mask <<= start;
        bits &= mask; 
        return bits;
    }
    else {
        errorMessage();
        exit(-1);
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

struct Cache *createCache(struct Cache *c, int numSetBits, int linesPerSet, int numBlockBits) {
    int i, j;
    c->numSetBits = numSetBits;
    c->numSets = (2 << numSetBits);
    c->linesPerSet = linesPerSet;
    c->numBlockBits = numBlockBits;
    c->blockSize = (2 << numBlockBits);
    c->numTagBits = 63 - numSetBits - numBlockBits;
    c->tags = malloc(c->numSets * c->linesPerSet * sizeof(unsigned long int));
    //initialize tags to -1
    for (i = 0; i < c->numSets; i++) {
        for(j = 0; j < c->linesPerSet; j++) {
            c->tags[i][j] = -1;
        }
    }
    
    return c;
}

int parseTraceFile(FILE * pf, char ** trace, struct Cache *cache) {
    int i;
    char buf[80];
    uint64_t address;
    char option;
    int size;
    int setNum, tag;     //blockNum;
    int setStartIndex = cache->numBlockBits + cache->numSetBits;
    int hit_count = 0;
    int miss_count = 0;
    int eviction_count = 0;

    pf = fopen(*trace, "r");
    if(!pf) {
        printf("Can't open trace file");
        return -1;
    }
    while(fgets(buf, 80, pf) != NULL) {
        if(buf[0] != ' ') {         //If the line begins with a space, we don't ignore it.
            sscanf(buf, "%c, %lu, %d", &option, &address, &size);
        }
        //blockNum = getBits(cache->numBlockBits, 63, address);
        setNum = getBits(setStartIndex, cache->numBlockBits + 1, address);
        tag = getBits(0, cache->numTagBits, address);
        for(i = 0; i < cache->linesPerSet; i++) {
           if(cache->tags[setNum][i] != 1 && tag == cache->tags[setNum][i]) {     //If it is valid and matches the tag
                if (option == 'M') {
                   hit_count += 2;
                }
                else hit_count++;
           }
        }
        miss_count++;
        for(i = 0; i < cache->linesPerSet; i++) {
            if(cache->tags[setNum][i] == -1) {
                cache->tags[setNum][i] = tag;
                if(option == 'M') {
                    hit_count++;
                }
            }
        }
        eviction_count++;
        for(i = 0; i < cache->linesPerSet; i++) {
            if(cache->tags[setNum][i] == tag) {
                if(option == 'M') {
                    hit_count++;
                }
            }
        }
    }
    return 0;

}

int parseCommandLine(int argc, char **argv, int *cache_s, int *cache_E, int *cache_b, char **trace, int *vflag) {
    int c;
    int argCount = 0;
    while((c = getopt(argc, argv, "h::v::s:E:b:t:")) != -1) {
        switch(c) {
            case 'h': 
                helpFlag = 1;
                break;
            case 'v':
                verboseFlag = 1; 
                break;
            case 's':
                argCount++;
                *cache_s = atoi(optarg);
                break;
            case 'E':
                argCount++;
                *cache_E = atoi(optarg);
                break;
            case 'b':
                argCount++;
                *cache_b = atoi(optarg);
                break;
            case 't':
                argCount++;
                strcpy(*trace, optarg);
                break;
            default:
                errorMessage();
                exit(-1);
                break;
        }
    }
    if (argCount < 4) {
        errorMessage();
        exit(-1);
    }
    return 0;
}

int main(int argc, char **argv)
{
    int numSetBits = 0;
    int numLinesPerSet = 0;
    int blockOffsetBits = 0;
    struct Cache cache;
    struct Cache *cacheP = &cache;
    int hit_count = 0;
    int miss_count = 0;
    int eviction_count = 0;
    FILE pf;
    char *trace = malloc(20);
    int vflag;
    parseCommandLine(argc, argv, &numSetBits, &numLinesPerSet, &blockOffsetBits, &trace, &vflag);
    cacheP = createCache(cacheP, numSetBits, numLinesPerSet, blockOffsetBits);
    parseTraceFile(&pf, &trace, cacheP);
    printSummary(hit_count, miss_count, eviction_count);
    return 0;
}
