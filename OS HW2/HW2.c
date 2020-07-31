/* Name = David Moore. Honor Pledge = UNC Honor Pledge: I certify that no unauthorized assistance has been received or
given in the completion of this work */
 
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#define MAX 10
#define LENGTH 100

void appendString(char* string, char charToAppend) {
        int lengthOfString = strlen(string);
        // maybe throw error here if  we extend past sizeOf(string);?
        string[lengthOfString] = charToAppend;
        string[lengthOfString+1] = '\0';
}

void newAppendFunction(char* string, char charToAppend) {
        
}


void clearArray(char* array) {
        memset(array, 0, sizeof(array));
}



void shiftArrayToLeft(char* array, int amountToShift) {
        int i;
        int arraySize = sizeof(array)/sizeof(array[0]);
        char result[arraySize];
        for (i = 0; i < arraySize; i++) {
                result[i] = array[(i + 1) % arraySize];
        }
}

main() {
        
        int flagToRepeat = 0;
        int flagToBreakOut = 0;
        int childPID;
        char *arrayOfArgumentsToExec[64];
        char input[MAX];
        // Get input from std.io:
        beginning:
        while(1) {
                // beginning:
                printf("%% ");
                // Declare arrays of words and clear them:
                // we need to clear the array to make sure it is purged of input from the last loop.
                clearArray(input);
                
                int tooManyCharacters = 0;
                int EOFFlag = 0;
                while(1) {
                        char newChar = getchar();
                                                // now I have input from stdin in input.
                        // check the input, and append it to the input array if needed.
                        if(tooManyCharacters >= 200) {
                                fprintf(stderr, "\nError, too many characters!\n");
                                while(1) {
                                        // loop until newline or EOF. 
                                        // If EOF, exit.
                                        newChar = getchar();
                                        if (newChar == EOF) {
                                                fprintf(stderr, "\nFound EOF while over character limit.\n Exiting!");
                                                exit(0);
                                        }
                                        // If newline, goTo beginning;
                                        if (newChar == '\n') {
                                                fprintf(stderr, "Found newline while over character limit. Going to Beginning \n");
                                                goto beginning;
                                        }
                                }
                                goto beginning;
                                
                                flagToBreakOut = 1;
                                break;
                        }

                        if(newChar == EOF) {
                                newChar = '\n';
                                appendString(input, newChar);
                                EOFFlag = 1;
                                break;                        
                        }
                        if (newChar == '\n') {
                                // we have reached end of this command line input.
                                appendString(input, newChar); 
                                break;
                        } 
                        // else:
                    appendString(input, newChar);
                    tooManyCharacters++;
                }
                if (flagToBreakOut == 1) {
                        flagToBreakOut = 0;
                        
                }
                // handle parsing of input array into strings of words to pass to command. 
                // Throw error if too many words? 
                
                // parse Input:
                char * pchar;
                pchar = strtok (input," \n\t");
                char arrayOfStrings[LENGTH][LENGTH];
                int j = 0;
                for(j = 0; j < LENGTH; j++) {
                        clearArray(arrayOfStrings[j]);
                                        }
                int arrayCounter = 0;
                while (pchar != NULL){
                        strcpy(arrayOfStrings[arrayCounter], pchar);
                        pchar = strtok (NULL, " \n\t");
                        arrayCounter++;
                }

                // Done with input.
        
                // Now arrayOfStrings is filled with arrays of each word from the original string, split by newlines, tabs, and spaces!!!
                // Now to do the fork and exec stuff!
                // Pretty Print arrayOfStrings:

                
                // try to move to new array:
               
                char** finalArray = malloc(101 * sizeof finalArray[0]);
                int counterNumber;
                for(counterNumber = 0; counterNumber<arrayCounter; counterNumber++) {
                        finalArray[counterNumber] = arrayOfStrings[counterNumber];
                }
                finalArray[arrayCounter] = NULL;

                childPID = fork();
                if (childPID < 0) {
                        // Error occured when creating child
                        fprintf(stderr,"Error occured when creating child process! Restarting.");
                        exit(0);
                } else if(childPID == 0) {
                        // exec:
                        int execStatusCode;
                
                        execStatusCode = execvp(finalArray[0], finalArray);
                                                int errsv = errno;
                        if(execStatusCode <0) {
                                fprintf(stderr, "Error occured attempting to Exec process. Restarting.");
                                exit(0);
                        }
                }else {
                        // Parent Code:
                        int statusOfChild;
                        free(finalArray);
                        waitpid(childPID, statusOfChild, 0);
                        if(statusOfChild == 1) {
                                fprintf(stderr, "Error! Child terminated with an error. Restarting.");
                                goto beginning;
                        }
                        // kill child:
                        
                        if (EOFFlag == 1) {
                                exit(0);
                        }
                }
        }
}