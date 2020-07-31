/* Name = David Moore. Honor Pledge = I have neither given nor received unauthorized aid on this assignment. I did reference the professor's provided example mmap code as well as these sources on MMAPS: https://medium.com/i0exception/memory-mapped-files-5e083e653b1 and https://w3.cs.jmu.edu/kirkpams/OpenCSF/Books/cs361/html/Mmap.html */
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h> 
#include <stdint.h>
#include <stddef.h>
#include <sys/mman.h>

#include "buffer.h"

#define SIZEOFOUTPUT    80
#define ERROR   -1

// SIZE of output should be 80 for actual submission.


void waitForChildren(pid_t* childpids){
    int status;
    while(ERROR < wait(&status)){ //Here the parent waits on any child.
        if(!WIFEXITED(status)){ //If the termination err, kill all children.
            printf("Got error. Killing all children");
            kill(childpids[0], SIGKILL);
            kill(childpids[1], SIGKILL);
            kill(childpids[2], SIGKILL);
            break;
        }
    }
}

void deleteMMAP(void* addr){
    //This deletes the memory map at given address. see man mmap
    if (ERROR == munmap(addr, sizeof(buffer))){
        perror("error deleting mmap");
        exit(EXIT_FAILURE);
    }
}


pid_t forkChild(void (*function)(buffer **), buffer** state){
    //This function takes a pointer to a function as an argument
    //and the functions argument. It then returns the forked child's pid.
    pid_t childpid;
    switch (childpid = fork()) {
        case ERROR:
            perror("fork error");
            exit(EXIT_FAILURE);
        case 0:
            (*function)(state);
        default:
            return childpid;
    }
}

pid_t forkChildSpecial(void (*function)(buffer **, buffer **), buffer** state1, buffer** state2){
    //This function takes a pointer to a function as an argument
    //and the functions argument. It then returns the forked child's pid.
    pid_t childpid;
    switch (childpid = fork()) {
        case ERROR:
            perror("fork error");
            exit(EXIT_FAILURE);
        case 0:
            (*function)(state1, state2);
        default:
            return childpid;
    }
}

buffer* createMMAP(size_t size){ // change message object to be bounded bufffer.
    //These are the neccessary arguments for mmap. See man mmap.
    void* addr = NULL;
    int protections = PROT_READ|PROT_WRITE; //can read and write
    int flags = MAP_SHARED|MAP_ANONYMOUS; //shared b/w procs & not mapped to a file
    int fd = -1; //We could make it map to a file as well but here it is not needed.
    off_t offset = 0;

    //Create memory map
    buffer* state = mmap(addr, size, protections, flags, fd, offset);

    if (( void *) ERROR == state){//on an error mmap returns void* -1.
        perror("error with mmap");
        exit(EXIT_FAILURE);
    }
    return state;
}


void inputProducerThread(buffer** boundedBuffer1) { 
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
            deposit(*boundedBuffer1, tempChar);
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
                deposit(*boundedBuffer1, tempChar);
                deposit(*boundedBuffer1, EOF);
            } else {
                // shelf was never initalized. Post EOF.
                deposit(*boundedBuffer1, EOF);

            }

            // return;
            exit(0);
        }
        
    }
}

void processProducerAndConsumerThread(buffer** boundedBuffer1, buffer** boundedBuffer2) {
    // do something
    char c;
    int flagToReturn = 0;
    while (true) {
    
        c = remoove(*boundedBuffer1);

        // check if char is a newline. Make it a space.
        if(c == '\n' || c == '\t'){
            c = ' ';
        } else if (c == EOF) {
            // pass and return;
            flagToReturn = 1;
        }

        // Now try to produce the new char into the buffer
        deposit(*boundedBuffer2, c);
        // done.
        if(flagToReturn != 0) {
            // return
            exit(0);
        }

    }
}

void outputConsumerThread(buffer** boundedBuffer2) {
    // output when 80 characters plus \n
    char character;
    char localCharArray[SIZEOFOUTPUT];
    int localArrayIndex = 0;
    while (true) {
        

        character = remoove(*boundedBuffer2);
        if(character == EOF) {
            // done
            // return;
            exit(0);
        }


        // now that we have the character, put it into an array.
        localCharArray[localArrayIndex] = character;
        localArrayIndex++;

        if(localArrayIndex == SIZEOFOUTPUT) {
            // we just filled up index 79 of the array
            // fprintf(stderr, "We have filled up final buffer in output process\n");
            // printf("\n@@@@We have filled up final buffer in output process@@@\n");
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
    // async maps?
    buffer* boundedBuffer2 = createMMAP(sizeof(buffer)); // reserve space in MMap memory for this buffer
    buffer* boundedBuffer1 = createMMAP(sizeof(buffer)); // reserve space in MMap memory for this buffer
    // printf("\nCreated MMAP\n");
    msync(boundedBuffer1, sizeof(buffer), 1); // set MMAP to be asyncronous because I have my own semaphores for syncronization
    msync(boundedBuffer2, sizeof(buffer), 1); // set MMAP to be asyncronous because I have my own semaphores for syncronization
    bufferInit(boundedBuffer1); // pass in boundedBuffer Don't need to malloc in init; 
    bufferInit(boundedBuffer2); // Init starting values for semaphores and character arrays with this function.

    buffer** bufferPointer1 = &boundedBuffer1; // Make pointers to buffer pointers. 
    buffer ** bufferPointer2 = &boundedBuffer2;
    // fork children:
    // need to have special case for processProducerAndConsumerThread for forkChild to take 2 bounded buffers.
    pid_t childpids[3];
    // printf("\nAbout to Fork\n");
    childpids[0] = forkChild(inputProducerThread, bufferPointer1); // Fork processes and have those new processes execute the previous thread calls as functions. Pass in my pointer to my buffer pointers so I make sure to modify the MMAP reference to my buffer instead of the local value.
    childpids[1] = forkChildSpecial(processProducerAndConsumerThread, bufferPointer1, bufferPointer2);
    childpids[2] = forkChild(outputConsumerThread, bufferPointer2);

    waitForChildren(childpids); // Block until children exit or have an error. 

    deleteMMAP(boundedBuffer1);    // Delete the MMAP mempory address since I don't need it now that my program is exiting. 
    deleteMMAP(boundedBuffer2);

    freeMeFromThisCurse(boundedBuffer1); // free the semaphore memory inside of my bounded buffers. 
    freeMeFromThisCurse(boundedBuffer2);

        
    exit(0); // DONE!
    
}