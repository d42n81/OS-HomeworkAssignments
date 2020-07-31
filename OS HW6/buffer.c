/*Created by David Moore. I have neither given nore received aid on this assignment.*/
// include libraries here.
#include "buffer.h"
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#define BSIZE  80
// char bufferArray[1];
// use malloc to define struct.
// define properties of struct. 
// at the end, have a method to free the memory of the adt. 
// Look up queue adt for reference. 

// If you have char* notation you do need to malloc
// But need to initialize buffer in main program with malloc. Define itialize(returns buffer pointer) and delete buffer 
// maybe make methods to create and destroy array and semaphore. 


// Need to have a size of buffer greater than 1; Buffer needs to at least be 2. 
void deposit(buffer *b, char c) { // maybe issue is that I'm  passing in the literal buffer. Need to pass the pointer
        //DO something
        // printf("\nIn buffer deposit(). We are depositing: %c", c);
        // sem_t emptyBuffers = b->emptyBuffers;
        sem_wait(&(b->emptyBuffers));
        sem_wait(&(b->mutex));
        //critical section
        // printf("\nWe are depositing: %c at NextIn = %d\n", c, b->nextIn);
        b->array[b->nextIn] = c;
        b->nextIn++;
        if (b->nextIn == BSIZE){b->nextIn = 0;}
        sem_post(&(b->mutex));
        sem_post(&(b->fullBuffers));
// semaphore
}

char remoove(buffer* b) { // pass in pointer
        // Do Something 
        // printf("\nIn buffer remoove()!");
        sem_wait(&(b->fullBuffers));
        sem_wait(&(b->mutex));
        
        // printf("In Critical Section of Remoove()");
        char c = b->array[b->nextOut];
        // printf("\nWe are remooving: %c at NextOut = %d\n", c, b->nextOut);
        b->nextOut++;
        if(b->nextOut == BSIZE){b->nextOut = 0;}

        sem_post(&(b->mutex));
        sem_post(&(b->emptyBuffers));

        return c;
//sem
}
void bufferInit(buffer *b) { // passed in pointer to buffer
        // printf("\nIn bufferInit()!");
        // buffer *b = malloc(sizeof(buffer));
        // b->array = array;
        b->nextIn = 0;
        b->nextOut = 0;
        sem_init(&(b->mutex),1,1);
        sem_init(&(b->fullBuffers), 1, 0);
        sem_init(&(b->emptyBuffers),1,BSIZE);
}

void freeMeFromThisCurse(buffer  *b) {
        // printf("In free method!!!");
        // free(b->array);
        sem_destroy(&(b->mutex));
        sem_destroy(&(b->emptyBuffers));
        sem_destroy(&(b->fullBuffers));
}