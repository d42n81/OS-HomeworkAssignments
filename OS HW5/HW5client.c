/* Name = David Moore. No unauthorized assistance was given or received. I did use Professor's given ToUpperClient.c as a reference for my code.*/
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>

#include "Socket.h"
#define MAXSIZE 3000
// MAX SIZE WAS 1024
int flagToBreakOut = 0;
int main(int argc, char *argv[]) {
    // printf("\nTop of Main \n");
    // main program code;
    char* hostName = argv[1];
    // strcpy(hostName, argv[1]);
    int port = atoi(argv[2]);
    

    int i, character, charCode;
    int whileCount = 0;
    int count = 0;
    int tooManyCharacters = 0; // keeps count of number of characters sent to the server. If over 500, expect to receive an error response back from the server.

    char lineData[MAXSIZE];
    // printf("\nConnecting to Socket \n");

    Socket connectSocket = Socket_new(hostName, port);
    if (connectSocket < 0) {
        // printf("Error. Something went wrong trying to connect to the server. please try again.");
        exit(1);
    }
    // Now that we are connected to the socket, print prompt and get input in a loop:
    while (whileCount < 10) {
        // loop (new line);
        // printf("\nTop of prompt Loop\n");
        printf("%% ");
        while ((fgets(lineData, sizeof(lineData), stdin) != NULL)) {
            // read in data from stdin
            // How to detect EOF compared to newline?
            count = strlen(lineData) + 1;
            // printf("\nLine Data is: %s\n", lineData);

            for (i = 0; i < count; i++) {
                // transmit characters onto the socket:
                character = lineData[i];
                charCode = Socket_putc(character, connectSocket);
                tooManyCharacters++;
                // printf("\nCharacter was transmitted to server. Character is: %c \n", character);
            }
            // printf("\nDone sending line data. About to receive characters.\n");
            if(tooManyCharacters >= 480) {
                // send signal to server end of big input
                Socket_putc('\r', connectSocket);
            }
            tooManyCharacters = 0;
            for (i = 0; i < 10000; i++) {
                // receive characters from server.
                character = Socket_getc(connectSocket);
                // printf("\nReceived Character. Character is: %c \n", character);
                // check what the first characters are to indicate status code for an error? Then conditionally print out error messages.
                if(character == EOF) {
                    flagToBreakOut = 1;
                    // printf("\nEOF Found! Flag to break out set\n");
                    break;
                }
                // lineData[i] = character;
                if (character == '\0'){
                    // server is done outputting. 
                    // printf("\nGot a null character from server. Server is done transmitting?\n");
                    break;
                }
                printf("%c", character);
            }
            // printf("\n");
            // if(i == MAXSIZE) {
            //     lineData[i-1] = '\0';
            // }

            // output:
            // printf("\n%s\n", lineData);
            if(flagToBreakOut == 1) {
                Socket_close(connectSocket);
                exit(0);
            }
            printf("%% ");
        
        }
        whileCount++;
        // on EOF from input we go here.
        Socket_putc(EOF, connectSocket);
        Socket_close(connectSocket);
        exit(0);
    }
    // close socket connection.
    Socket_close(connectSocket);
}