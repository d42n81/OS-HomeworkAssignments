
// Name = David Moore. Honor Pledge = I have neither given nor received aid on this assignment.

#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
// C:\Users\d42n81\comp426-cli\a00\HW3WithBoundedBuffer.c
#include <stdlib.h> 
#include <stdint.h>
#include <stddef.h>

#include "buffer.h"

#define SIZEOFOUTPUT    80
// SIZE of output should be 80 for actual submission.

// declare Buffers
    // char inputToProcessBuffer [SIZE];
    // char processToOutputBuffer [SIZE];

    char * charArray1;
    char * charArray2;

    buffer boundedBuffer1;
    buffer boundedBuffer2;
    
//Declare Semaphores:
    // sem_t mutex1;
    // sem_t fullBuffer1;
    // sem_t emptyBuffer1;
    // sem_t mutex2;
    // sem_t fullBuffer2;
    // sem_t emptyBuffer2;

void *inputProducerThread(void *arg) { 
    // take in input from stdinput. 
    /*Gets char from stdinput. IF CRLF, pass it on and join. If letter, pass it and not done. If
    * asterisk, shelve it. Pull next. If next is asterisk, make carrot. 
    */
    // char[80] localCharArray;
    // printf("In first thread!\n");
    char shelf = '\0';
    int pullPointer = 0;
    int pushPointer = 0;
    bool done = false;
    char tempChar;
    int switchShelfFlag = 0;
    
    while (true) {
        // get input:
        tempChar = getchar();
        // printf("Got char: %c", tempChar);
        if(tempChar != EOF) {
            // process input
            if(tempChar == '*') {
                // check shelf
                if(shelf == '*') {
                    tempChar = '^';
                    shelf = '\0';
                } else {
                    if(shelf == '\0') {
                        // initialize shelf. 
                        shelf = '*';
                        // printf("\n Initialized Shelf! \n");
                        continue;
                    } else {
                        char charToSwitch = shelf;
                        shelf = tempChar;
                        tempChar = charToSwitch;
                        // temp char is now a character that is not an asterisk or \0
                        switchShelfFlag = 1;
                    }
                    
                }
            }
            // continue after changing ** to ^
            if(shelf != '\0' && switchShelfFlag == 0) {
                // shelf has been initialized. make that temp char.
                char charToSwitch = shelf;
                shelf = tempChar;
                tempChar = charToSwitch;
            }
            // continue after checking shelf:
            // push tempchar onto buffer:
           //  printf("\nAbout to deposit(): %c", tempChar);
            deposit(&boundedBuffer1, tempChar);
            // printf("\nJust deposited: %c in bounded buffer1\n", tempChar);
            // done;
            // done;
            switchShelfFlag = 0;

        } else {
            // pass EOF and return;
            // check shelf:
            if(shelf != '\0') {
                // shelf has been initialized. make that temp char.
                char charToSwitch = shelf;
                shelf = tempChar;
                tempChar = charToSwitch;

                // produce tempChar and EOF:
                // printf("\nAbout to deposit(): %c", tempChar);
                deposit(&boundedBuffer1, tempChar);
                // printf("\nJust deposited: %c in bounded buffer1\n", tempChar);

                // printf("\nAbout to deposit() EOF");
                deposit(&boundedBuffer1, EOF);
                // printf("\nJust deposited: EOF in bounded buffer1\n");
            } else {
                // shelf was never initalized. Post EOF.
                // printf("\nAbout to deposit() EOF");
                deposit(&boundedBuffer1, EOF);
                // printf("\nJust deposited: EOF in bounded buffer1\n");

            }

            return;
        }
        
    }
}

void *processProducerAndConsumerThread(void *arg) {
    // do something
    // printf("In Second Thread!!!");
    char c;
    int flagToReturn = 0;
    while (true) {
        // printf("\nIn second Thread, trying to remoove");
        c = remoove(&boundedBuffer1);
        // printf("\nJust remooved: %c from bounded buffer1\n", c);
        // printf("\nIn second Thread. Just tried to remoove. Got:%c", c);

        // check if char is a newline. Make it a space.
        if(c == '\n' || c == '\t'){
            c = ' ';
        } else if (c == EOF) {
            // pass and return;
            flagToReturn = 1;
        }

        // Now try to produce the new char into the buffer
        deposit(&boundedBuffer2, c);
        // printf("\nJust deposited: %c in bounded buffer2\n", c);
        // done.
        if(flagToReturn != 0) {
            // return
            return;
        }

    }
}

void *outputConsumerThread(void *arg) {
    // output when 80 characters plus \n
    // printf("\nIn third thread!!!");
    char character;
    char localCharArray[SIZEOFOUTPUT];
    int localArrayIndex = 0;
    while (true) {
        // consume character
        // printf("\nI'm in the third thread and trying to consume a character.");

        character = remoove(&boundedBuffer2);
        // printf("\nJust remooved: %c from bounded buffer2\n", character);
        if(character == EOF) {
            // done
            return;
        }

        // printf("\n I'm in the third thread and just consumed a character: %c", character);

        // now that we have the character, put it into an array.
        localCharArray[localArrayIndex] = character;
        localArrayIndex++;

        if(localArrayIndex == SIZEOFOUTPUT) {
            // we just filled up index 79 of the array.
            // print, set the local index back down to 0;
            for(int i = 0; i <SIZEOFOUTPUT; i++) {
                printf("%c", localCharArray[i]);
            }
            printf("%c", '\n');
            localArrayIndex = 0;
        }
    }
}

int main() {

        // printf("In main()");

    // Initialize buffers:
    // charArray1 = malloc(80*sizeof(char));
    // charArray2 = malloc(80*sizeof(char));
    bufferInit(&boundedBuffer1); // pass in &boundedBuffer Don't need to malloc in init; 
    bufferInit(&boundedBuffer2);
    // declare Threads:
    pthread_t t1, t2, t3;

    pthread_create(&t1, NULL, inputProducerThread, NULL);
    pthread_create(&t2, NULL, processProducerAndConsumerThread, NULL);
    pthread_create(&t3, NULL, outputConsumerThread, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);

    freeMeFromThisCurse(&boundedBuffer1);
    freeMeFromThisCurse(&boundedBuffer2);

    //printf("Done in main method");
        
    return 0;
    
}