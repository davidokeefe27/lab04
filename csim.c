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

typedef struct {
    int numSets;
    int numSetBits;
    int linesPerSet;
    int numBlockBits;
    int blockSize;
    int numTagBits;
    unsigned long int ** tags;
}Cache;

//Function Declarations
int parseCommandLine(int argc, char **argv, int *cache_s, int *cache_E, int *cache_b, char **trace, int *vflag);
uint64_t getBits(int start, int end, unsigned long bits);
void errorMessage();
Cache * createCache(int numSetBits, int linesPerSet, int numBlockBits);
int parseTraceFile(FILE * pf, char ** trace, Cache *cache, int * h_c, int * m_c, int * e_c);
void insertAndAdjustLRU(Cache * cache, unsigned long int tag, int setNum);
//int processM(unsigned long int, int, int, Cache *);
//int processLS(unsigned long int, int, int, Cache *);
int findDuplicateTag(Cache * cache, int tag, int setNum);
int findEmptyLine(Cache * cache, int setNum);

//Global Variable(s)
int verboseFlag = 0;
int helpFlag = 0;

void errorMessage() { 
    printf("Error\n");
    printf("Usage: ./csim-ref [-h] [-v] -s <s> -E <E> -b <b> -t <tracefile>\n");
}

/*
 * Scans every line in a particular set of the cache.
 * Finds whether a tag is already present in the cache.
 *
 * Params: cache pointer, tag, set number.
 * Returns: index of matched tag if a match is found. -1 if tag is not already present.
 */
int findDuplicateTag(Cache * cache, int tag, int setNum) {
    int i;
    for(i = 0; i < cache->linesPerSet; i++) {
        if(cache->tags[setNum][i] == tag) {
            return i;
        }
    }
    return -1;
}
/*
 * Scans every line in a set and returns the line index number if one is found.
 *
 * Params:  cache pointer, set Number to iterate through
 * Returns: -1 if no line is empty.
 *          Line index if empty line is found.
 */
int findEmptyLine(Cache * cache, int setNum) {
    int i;
    for(i = 0; i < cache->linesPerSet; i++) {
        if(cache->tags[setNum][i] == -1) return i;
    }
    return -1;
}
/*
int processM(unsigned long int address, int startIndex, int endIndex, Cache * cache) {
    int setNum, i;
    int numHits = 0;
    unsigned long int tag;
    tag = getBits(startIndex, endIndex, address);
    for(i = 0; i < cache->linesPerSet; i++) {
        if(cache->tags[setNum][i] != -1 && cache->tags[setNum][i] == tag) {
            numHits++;
            insertAndAdjustLRU(cache, tag, setNum);
        }
    }
    for(i = 0; i < cache->linesPerSet; i++) {
        if(cache->tags[setNum][i] == tag) {
            numHits++;
        }
    }
    return numHits;
}

int processLS(unsigned long int address, int startIndex, int endIndex, Cache * cache) {
    int setNum, i;
    int numHits = 0;
    unsigned long int tag;
    tag = getBits(startIndex, endIndex, address);
}
*/
/*
 * function for getting the bits from the start to the end of an unsigned long int
 *
 * Params: start and end indices, the address.
 * Returns: bits from the ranges specified.
 *
 */
uint64_t getBits(int start, int end, unsigned long address) {
    int i;
    unsigned long mask = 0UL;
    if(start < 64 && start >= 0 && end < 64 && end >= 0) {
        int diff = end - start;
        for (i = 0; i < diff; i++) {
            mask <<= 1;
            mask |= 1;
        }
        mask <<= start;
        address &= mask; 
        return address;
    }
    else {
        errorMessage();
        printf("getBits\n");
        exit(-1);
    }
}

/*
 * Function to fill in a cache structure's information that is extracted from the command line
 *
 * Params: cache pointer, s, E, and b
 *
 */
Cache * createCache(int numSetBits, int linesPerSet, int numBlockBits) {
    int i, j;
    //allocate memory for the cache structure
    Cache * c_ptr = malloc(sizeof(* c_ptr));
    if(c_ptr != NULL) {
        c_ptr->numSetBits = numSetBits;
        c_ptr->numSets = (int)pow(2, numSetBits);
        c_ptr->linesPerSet = linesPerSet;
        c_ptr->numBlockBits = numBlockBits;
        c_ptr->blockSize = (int)pow(2, numBlockBits);
        c_ptr->numTagBits = 63 - numSetBits - numBlockBits;
        //allocate memory for the 2D tags array
        c_ptr->tags = malloc(c_ptr->numSets * linesPerSet * sizeof(unsigned long int));
        c_ptr->tags = malloc(sizeof(unsigned long int) * c_ptr->numSets);
        for(i = 0; i < c_ptr->numSets; i++) {
            c_ptr->tags[i] = malloc(sizeof(unsigned long int) * linesPerSet);
        }
        //initialize tags to -1
        for(i = 0; i < c_ptr->numSets; i++) {
            for(j = 0; j < linesPerSet; j++) {
                c_ptr->tags[i][j] = -1;
            }
        }
    }
    return c_ptr;    
}

/*
 * Function used to adjust the ordering of the lines in the set
 *  to keep them in least recently used order and then insert the
 *  new tag into the beginning of the set, as it was the most recently
 *  used line.
 *
 *  Algorithm evicts the least recently used tag automatically if the set is full
 *
 *  Params: cache pointer, tag number, set number
 *
 */
void insertAndAdjustLRU(Cache * cache, unsigned long int tag, int setNum) {
    int i;
    int index;
    index = findDuplicateTag(cache, tag, setNum);

    //if the findDuplicateTag does not find an identical tag in the set
    if(index == -1) {
        //iterate through the lines in the set and move them all down 1
        for(i = cache->linesPerSet - 1; i > 1; i--) {
            cache->tags[setNum][i] = cache->tags[setNum][i - 1]; 
        }
        //then set the most recently used index of the array to the new tag
        cache->tags[setNum][0] = tag;
    }
    //else the tag was already present. just need to move the tag to the 
    //front of the array and move everything else down.
    else {
        for(i = index; i > 1; i--) {
            cache->tags[setNum][i] = cache->tags[setNum][i-1];
        }
        cache->tags[setNum][0] = tag;    
    }   
}

/*
 * Function to go through a trace file line by line and update the miss/hit/eviction counts based
 *  off of the information it extracts.
 *
 *  Params: file pointer, tracefile, and then pointers for: cache, hit count, miss count, eviction count.
 *  Return: -1 if error. 0 if not.
 *
 */
int parseTraceFile(FILE * pf, char ** traceFile, Cache * cache, int * h_c, int * m_c, int * e_c) {
    int i;
    char buf[80];
    unsigned long int address;
    char option;
    int size, setNum;
    unsigned long int tag;
    int setEndBit = cache->numBlockBits + cache->numSetBits;

    pf = fopen(* traceFile, "r");
    if(!pf) {
        printf("Can't open trace file\n");
        return -1;
    }
    while(fgets(buf, 80, pf) != NULL) {
        if(buf[0] == ' ') {         //If the line begins with a space, we don't ignore it.
            sscanf(buf, " %c %lu,%d", &option, &address, &size);
        }
        printf("%s", buf);
        setNum = getBits(cache->numBlockBits, setEndBit - 1, address);
        tag = getBits(setEndBit, 63, address);
        printf("numBlockBits: %d, setEndBit: %d, address: %lu\n", cache->numBlockBits, setEndBit, address);
        printf("numTagBits: %d\n", cache->numTagBits);
        printf("tag: %lu\n", tag);
        printf("setNum: %d\n", setNum);
        printf("option: %c\n", option);
        for(i = 0; i < cache->linesPerSet; i++) {
           printf("first for loop\n");
           if(tag == cache->tags[setNum][i]) { //If it is valid and matches the tag
               printf("1\n");
               printf("option: %c\n", option);
                if (option == 'M') {
                   *h_c = *h_c + 2;
                }
                else *h_c = *h_c + 1;
                printf("hit_count: %d\n", *h_c);
                insertAndAdjustLRU(cache, tag, setNum);
                break;
           }
        }
        *m_c = *m_c + 1;
        for(i = 0; i < cache->linesPerSet; i++) {
            if(cache->tags[setNum][i] == -1) {
                cache->tags[setNum][i] = tag;
                if(option == 'M') {
                    *h_c = *h_c + 1;
                }
                insertAndAdjustLRU(cache, tag, setNum);
                break;
            }
        }
        *e_c = *e_c + 1;
        for(i = 0; i < cache->linesPerSet; i++) {
            if(cache->tags[setNum][i] == tag) {
                if(option == 'M') {
                    *h_c = *h_c + 1;
                }
                insertAndAdjustLRU(cache, tag, setNum);
                break;
            }
        }
    }
    return 0;

}

/* 
 * Function for extracting information from the command line. 
 *
 * Uses getopt() for parsing
 * 
 * Parameters are argc and the argv array, cache info from the command line, the trace file name, and the verbose flag.
 * Returns 0 if no error. -1 if error.
 *
 */
int parseCommandLine(int argc, char ** argv, int * cache_s, int * cache_E, int * cache_b, char ** traceFile, int * vflag) {
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
                * cache_s = atoi(optarg);
                break;
            case 'E':
                argCount++;
                * cache_E = atoi(optarg);
                break;
            case 'b':
                argCount++;
                * cache_b = atoi(optarg);
                break;
            case 't':
                argCount++;
                strcpy(* traceFile, optarg);
                break;
            default:
                errorMessage();
                printf("default in switch statement in parseCommandLine\n");
                exit(-1);
                break;
        }
    }
    if (argCount < 4) {
        errorMessage();
        printf("argCount too low\nArgCount = %d\n", argCount);
        exit(-1);
    }
    return 0;
}



int main(int argc, char **argv)
{
    int numSetBits = 0;
    int numLinesPerSet = 0;
    int blockOffsetBits = 0;
    Cache cache;
    Cache * cacheP = &cache;
    int hit_count = 0;
    int miss_count = 0;
    int eviction_count = 0;
    FILE pf;
    char * traceFile = malloc(20);
    int vflag;
    parseCommandLine(argc, argv, &numSetBits, &numLinesPerSet, &blockOffsetBits, &traceFile, &vflag);
    cacheP = createCache(numSetBits, numLinesPerSet, blockOffsetBits);
    parseTraceFile(&pf, &traceFile, cacheP, &hit_count, &miss_count, &eviction_count);
    printSummary(hit_count, miss_count, eviction_count);
    return 0;
}
