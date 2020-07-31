// Name = David Moore. Honor Pledge = I have neither given nor received aid on this assignment.

#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define SIZE 1

// declare Buffers
    char inputToProcessBuffer [SIZE];
    char processToOutputBuffer [SIZE];
//Declare Semaphores:
    sem_t mutex1;
    sem_t fullBuffer1;
    sem_t emptyBuffer1;
    sem_t mutex2;
    sem_t fullBuffer2;
    sem_t emptyBuffer2;

void *inputProducerThread(void *arg) { 
    // take in input from stdinput. 
    /*Gets char from stdinput. IF CRLF, pass it on and join. If letter, pass it and not done. If
    * asterisk, shelve it. Pull next. If next is asterisk, make carrot. 
    */
    // char[80] localCharArray;
    char shelf = '\0';
    int pullPointer = 0;
    int pushPointer = 0;
    bool done = false;
    char tempChar;
    int switchShelfFlag = 0;
    
    while (true) {
        // get input:
        tempChar = getchar();
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
            sem_wait(&emptyBuffer1);
            sem_wait(&mutex1);
            //critical section
            inputToProcessBuffer[0] = tempChar;

            sem_post(&mutex1);
            sem_post(&fullBuffer1);
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
                sem_wait(&emptyBuffer1);
                sem_wait(&mutex1);
                //critical section
                inputToProcessBuffer[0] = tempChar;

                sem_post(&mutex1);
                sem_post(&fullBuffer1);

                
                sem_wait(&emptyBuffer1);
                sem_wait(&mutex1);
                //critical section
                inputToProcessBuffer[0] = EOF;

                sem_post(&mutex1);
                sem_post(&fullBuffer1);
            } else {
                // shelf was never initalized. Post EOF.
                sem_wait(&emptyBuffer1);
                sem_wait(&mutex1);
                //critical section
                inputToProcessBuffer[0] = EOF;

                sem_post(&mutex1);
                sem_post(&fullBuffer1);

            }

            return;
        }
        
    }
}

void *processProducerAndConsumerThread(void *arg) {
    // do something
    char c;
    int flagToReturn = 0;
    while (true) {
        sem_wait(&fullBuffer1);
        sem_wait(&mutex1);

        c = inputToProcessBuffer[0];

        sem_post(&mutex1);
        sem_post(&emptyBuffer1);

        // check if char is a newline. Make it a space.
        if(c == '\n' || c == '\t'){
            c = ' ';
        } else if (c == EOF) {
            // pass and return;
            flagToReturn = 1;
        }

        // Now try to produce the new char into the buffer
        sem_wait(&emptyBuffer2);
        sem_wait(&mutex2);

        // critical section, post to buffer:
        processToOutputBuffer[0] = c;

        sem_post(&mutex2);
        sem_post(&fullBuffer2);
        // done.
        if(flagToReturn != 0) {
            // return
            return;
        }

    }
}


void *outputConsumerThread(void *arg) {
    // output when 80 characters plus \n
    char character;
    char localCharArray[80];
    int localArrayIndex = 0;
    while (true) {
        // consume character

        sem_wait(&fullBuffer2);
        sem_wait(&mutex2);

        character = processToOutputBuffer[0];

        sem_post(&mutex2);
        sem_post(&emptyBuffer2);

        // now that we have the character, put it into an array.
        localCharArray[localArrayIndex] = character;
        localArrayIndex++;

        if(localArrayIndex == 80) {
            // we just filled up index 79 of the array.
            // print, set the local index back down to 0;
            for(int i = 0; i <80; i++) {
                printf("%c", localCharArray[i]);
            }
            printf("%c", '\n');
            localArrayIndex = 0;
        }

    }
}

int main() {

    // Initialize semaphores:
    sem_init(&mutex1, 0, 1);
    sem_init(&mutex2, 0, 1);

    sem_init(&fullBuffer1, 0, 0);
    sem_init(&emptyBuffer1, 0, 1);

    sem_init(&fullBuffer2, 0, 0);
    sem_init(&emptyBuffer2, 0, 1);
    // declare Threads:
    pthread_t t1, t2, t3;

    pthread_create(&t1, NULL, inputProducerThread, NULL);
    pthread_create(&t2, NULL, processProducerAndConsumerThread, NULL);
    pthread_create(&t3, NULL, outputConsumerThread, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);

    sem_destroy(&mutex1);
    sem_destroy(&mutex2);
    sem_destroy(&fullBuffer1);
    sem_destroy(&emptyBuffer1);
    sem_destroy(&fullBuffer2);
    sem_destroy(&emptyBuffer2);

    return 0;
    
}
