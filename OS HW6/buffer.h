/*Created by David Moore. Honor Pledge = No outside help was given or received on this assignment.*/
// #include <stdio.h>
// #include <stdbool.h>
#include <semaphore.h>
// Maybe do the #ifndef #endif 
#ifndef BOUNDEDBUFFER
#define BOUNDEDBUFFER

// ADT is like a class.  Methods are gonna be functional. 
// Need to capture all of the abstractions that make it a bounded buffer. Need a struct. \
// Need to encapsulate array inside of ADT. 
// TA is looking for all of the declarations to be in this header file. 
// The struct's definition is the struct. Need to include all the members of the struct. 
#define SIZE    1

typedef struct buffer {// properties go here, like the array, next in, next out.
        char array[80]; // char array[size];
        int nextIn;
        int nextOut;
        // need more for bounded buffer.
        // need to make semaphores be held here. 
        sem_t mutex; 
        sem_t emptyBuffers;
        sem_t fullBuffers;
} buffer;

void deposit (buffer *b, char c);

char remoove(buffer *b);

void bufferInit(buffer * b);

void freeMeFromThisCurse(buffer * b);
#endif