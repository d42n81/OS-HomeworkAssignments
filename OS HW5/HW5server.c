/* Name = David Moore. No unauthorized assistance was given or received. I did use Professor's given ToUpperDaemon.c and the ForkExecRedirect.c as a reference for my code.*/
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include "Socket.h"
#define MAXSIZE 1024
// MAX SIZE of 1024 was working
#define MAXTMP 100

// Define Globals:

ServerSocket welcomeSocket;
Socket connectionSocket;
char lineData[MAXSIZE];
int count;
int character; 
unsigned char newLine[MAXSIZE];

unsigned char tmpName[MAXTMP];
unsigned char idString[MAXTMP];
int id;
FILE *tmpFP;
FILE *fp;
pid_t childPID;
pid_t termPID;
int childStatus;
int flagToBreakOut = 0;
int lengthOfLineData = 0;

void clearArray(char* array) {
        memset(array, 0, sizeof(array));
}
void cleanUpFunction() {
    Socket_close(connectionSocket);
    Socket_close(welcomeSocket);
    fclose(tmpFP);
    remove(tmpName);
}

int main(int argc, char *argv[]) {
    // code:
    atexit(cleanUpFunction);

    // connect to Socket:
    int port = atoi(argv[1]);
    count = 0;
    welcomeSocket = ServerSocket_new(port);
    connectionSocket = ServerSocket_accept(welcomeSocket);
    int tooManyCharacters = 0;
    while(true) {
        beginning:
        // server code needs to loop forever?
        // get characters from socket.
        
        for(int i = 0; i < MAXSIZE; i++) {
            if(tooManyCharacters >= 479){
                fprintf (stderr, "TOO MANY CHARACTERS\n");
                // Loop until newline or EOF.
                while(true) {
                    character = Socket_getc(connectionSocket);
                    if(character == EOF) {
                        fprintf (stderr, "Got EOF while over too many characters\n");
                        flagToBreakOut = 1;
                        // SEND ERROR MESSAGE;
                        Socket_close(connectionSocket);
                        Socket_close(welcomeSocket);
                        exit(0);
                        break;
                    }
                    if(character == '\r') {
                        fprintf (stderr, "Found Carriage Return while over too many characters\n");
                        // SEND ERROR MESSAGE
                        // RESTORE STATE
                        // GOTO BEGINNING
                        char *temporaryString = "ERROR. Too many characters in command. Try again.\n";
                        int temporary;
                        char currentChar;
                        for(temporary = 0; temporary<strlen(temporaryString); temporary++) {
                            currentChar = temporaryString[temporary];
                            Socket_putc(currentChar, connectionSocket);
                        }
                        Socket_putc('\0', connectionSocket);
                        // clearArray(lineData);
                        tooManyCharacters = 0;
                        goto beginning;

                    }
                }
                // Send back tooManyCharacters Error. 
                // GoTo beginning;
            }
            if (i == MAXSIZE-1) {
                // we are filled up. Add termination to string and break.
                lineData[i] = '\0';
                break;
            }
            character = Socket_getc(connectionSocket);
            tooManyCharacters++;
            if(character == EOF) {
                // will character ever be EOF?
                // ERROR!
                flagToBreakOut = 1;
                lineData[i] = '\0';
                break;
            } else {
                if(character == '\0') {
                    // reached the end of input?
                    lineData[i] = character;
                    break;
                }
                lineData[i] = character;
            }
        }
        lengthOfLineData = strlen(lineData);
        tooManyCharacters = 0;

        // redirect STDOUT to file:
        id = (int) getpid();
        sprintf(idString, "%d", id);
        strcpy(tmpName,"tmp");
        strcat(tmpName, idString);
        fp = freopen(tmpName, "w", stdout); // we write to this with our printf calls. 
        // read from tmpName file with tmpFP pointer later.

        // end of looping for data!
        // Line data should be filled up with all the data we can hold. 
        // Send status code back to client? 
        // Exec lineData into temp file. Then get ready to send that back to the client?
        // Get lineData in format we need:
        char * pchar;
        // printf ("Splitting string \"%s\" into tokens: \n\t",input);
        pchar = strtok (lineData," \n\t");
        char arrayOfStrings[100][100];
        int j = 0;
        for(j = 0; j < 100; j++) {
            clearArray(arrayOfStrings[j]);
        }
        int arrayCounter = 0;
        while (pchar != NULL){
            //printf ("%s\n",pchar);
            strcpy(arrayOfStrings[arrayCounter], pchar);
            pchar = strtok (NULL, " \n\t");
            arrayCounter++;
        }

        char** finalArray = malloc(101 * sizeof finalArray[0]);
        int counterNumber;
        for(counterNumber = 0; counterNumber<arrayCounter; counterNumber++) {
            // strcpy(finalArray, arrayOfStrings[arrayCounter]);
            finalArray[counterNumber] = arrayOfStrings[counterNumber];
        }
        finalArray[arrayCounter] = NULL;
        // now we can fork and do execvp(finalArray[0], finalArray);
        if(flagToBreakOut == 0 || lengthOfLineData != 0) {
            // only fork if we have a non null input;
            childPID = fork();
        }
        
        if(childPID == -1) {
            perror("fork");
            // exit(-1); ?
            goto beginning;
        }
        if(childPID == 0) {
            // child process code
            int execCode;
            execCode = execvp(finalArray[0], finalArray);
            if(execCode == -1) {
                perror("exec");
                // char* tempFinalArray = "Not a valid executable command";
                // execvp("echo", tempFinalArray);
                printf("Error. Not a valid executable command.\n");
                exit(1); 
                
            }
        }else {
            // parent process:
            if(flagToBreakOut == 0) {
                termPID = waitpid(childPID, &childStatus, 0);
            }
            free(finalArray);
            if(termPID == -1) {
                perror("waitpid");
            } else {
                // kill zombies. Print zombie status:
                if (WIFEXITED(childStatus) && (flagToBreakOut == 0 || lengthOfLineData != 0) && childStatus != 0) {
                    printf("Error! Not an executable command. PID %d exited, status = %d\n", childPID, WEXITSTATUS(childStatus));
                } else {
                    // printf("PID %d did not exit normally\n", childPID);
                    // kill child;
                    if(flagToBreakOut == 0 || lengthOfLineData != 0) {
                        kill(childPID, SIGTERM);

                    }
                }
            }
            // close write fp?
            fclose(fp);
            if ((tmpFP = fopen (tmpName, "r")) == NULL) {
                fprintf (stderr, "error opening tmp file\n");
                fprintf (stderr, "Tmp file is %s\n", tmpName);
                printf("Could not open up TMP File. Try again later.");
                // exit (-1); ?
                char *tempString = "Not a valid executable command. Try again.\n";
                int temp;
                char curChar;
                for(temp = 0; temp<strlen(tempString); temp++) {
                    curChar = tempString[temp];
                    Socket_putc(curChar, connectionSocket);
                }
                Socket_putc('\0', connectionSocket);
                // fclose(tmpFP);
                // remove(tmpName);
                unlink(tmpName);
                continue;
            }
            int countOfSentCharacters = 0;
            while (!feof (tmpFP)) {
                /* Get line from file */
                if(countOfSentCharacters >= 1015){
                    // if we are about to send more characters than the client can handle, stop sending.
                    Socket_putc('\0', connectionSocket);
                    break;
                }
                if (fgets (newLine, sizeof(newLine), tmpFP) == NULL) { // need to change this to read all the lines up to EOF, not newline.
                    Socket_putc('\0', connectionSocket);
                    countOfSentCharacters++;
                    break;
                }
                char tempChar;
                int charCode;
                // send tmpFile back to client over socket. 
                for(int k = 0; k < strlen(newLine); k++) {
                    tempChar = newLine[k];
                    charCode = Socket_putc(tempChar, connectionSocket);
                    countOfSentCharacters++;

                }
                // put EOF to show we are done?
                // Socket_putc('\0', connectionSocket);
            }
            
        }
        fclose(tmpFP);
        remove(tmpName);

        if (flagToBreakOut == 1) {
            Socket_close(connectionSocket);
            Socket_close(welcomeSocket);
            exit(0);
        }
        continue;
    }
    // End while Loop

}
// End main;