// Name = David Moore. Honor Pledge = I have neither given nor received aid on this assignment

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stddef.h>
#include <stdbool.h>
#define SIZEOFOUTPUT    80
// continue when I get null character doesn't work, because it brings me back to the very top of main. GoTo line?
// test to make sure my message passing at least works.
int main() {
    // execute main program.
    // might have problems passing eof?
    // Issue with passing single characters. Needs to be a string?

    // Declare 2 Pipes
    int inputToProcessPipe[2]; // write into 1, read from 0
    int processToOutputPipe[2]; // write into 1, read from  0
    pipe(inputToProcessPipe);
    pipe(processToOutputPipe);

    // fork 3 processes
    int inputProducerPID = fork();
    if(inputProducerPID == 0) {
        // input Producer code:
        char inputBuffer[1];
        close(inputToProcessPipe[0]);
        close(processToOutputPipe[0]);
        close(processToOutputPipe[1]);

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
                inputBuffer[0] = tempChar;
                write(inputToProcessPipe[1], inputBuffer, 1);

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
                    inputBuffer[0] = tempChar;
                    write(inputToProcessPipe[1], inputBuffer, 1);

                    inputBuffer[0] = EOF;
                    write(inputToProcessPipe[1], inputBuffer, 1);
                } else {
                    // shelf was never initalized. Post EOF.
                    inputBuffer[0] = EOF;
                    write(inputToProcessPipe[1], inputBuffer, 1);

                }

                exit(0);
            }
        
        }
        // end of loop

    } else {
        // fork the next process from parent
        int processProducerAndConsumerPID = fork();
        if(processProducerAndConsumerPID == 0) {
            // process Producer and Consumer code:


            // close unneeded pipe entrances
            close(inputToProcessPipe[1]);
            close(processToOutputPipe[0]);
            char inputBuffer[1];
            char outputBuffer[1];

            char c;
            int flagToReturn = 0;
            while (true) {
                read(inputToProcessPipe[0], inputBuffer, 1);
                c = inputBuffer[0];
                int charCode = (int) c;
                if(c == '\0') {
                    // NOOP
                    continue;
                }
               

                // check if char is a newline. Make it a space.
                if(c == '\n' || c == '\t'){
                    c = ' ';
                } else if (c == EOF) {
                    // pass and return;
                    flagToReturn = 1;
                }

                // Now try to produce the new char into the buffer
                outputBuffer[0] = c;
                write(processToOutputPipe[1], outputBuffer, 1);
                // done.
                if(flagToReturn != 0) {
                    // return
                    exit(0);
                }

            }
            // end of while

        } else {
            // fork the next process from parent:
            int outputConsumerPID = fork();
            if(outputConsumerPID == 0) {
                // output consumer code: 
                // char outputBuffer[80];
                close(inputToProcessPipe[0]);
                close(inputToProcessPipe[1]);
                close(processToOutputPipe[1]);
                char inputBuffer[1];

                char character;
                char localCharArray[SIZEOFOUTPUT];
                int localArrayIndex = 0;
                while (true) {
                    // consume character

                    read(processToOutputPipe[0], inputBuffer, 1);
                    character = inputBuffer[0];
                    // printf("\nIn output. Just Read. Character is: %c\n", character);
                    int charCode = (int) character;
                    if(character == '\0') {
                        // NOOP
                        continue;
                    }
                    if(character == EOF) {
                        // done
                        exit(0);
                    }


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

            } else {
                // Parent Code!!!!
                // Execute main parent code by having it wait until the processes have finished;
                int statusOfInput;
                int statusOfProcessor;
                int statusOfOutput;

                waitpid(inputProducerPID, statusOfInput, 0);
                waitpid(processProducerAndConsumerPID, statusOfProcessor, 0);
                waitpid(outputConsumerPID, statusOfOutput, 0);



            }
        }
    }


}