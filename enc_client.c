/*
* Max Baker
* enc_client.c 
* March 20th, 2024
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()

/**
* Client code
* 1. Create a socket and connect to the server specified in the command arugments.
* 2. Prompt the user for input and send that input as a message to the server.
* 3. Print the message received from the server and exit the program.
*/

const char* id = "enc_client"; //used to identify the program to the server

// Error function used for reporting issues
void error(const char *msg) { 
    perror(msg); 
    exit(0); 
} 

// Set up the address struct
void setupAddressStruct(struct sockaddr_in* address, int portNumber){
 
    // Clear out the address struct
    memset((char*) address, '\0', sizeof(*address)); 

    // The address should be network capable
    address->sin_family = AF_INET;
    // Store the port number
    address->sin_port = htons(portNumber);
}

void check_for_bad_chars(char* plaintext){
    //checks if the plaintext input has any characters that shouldn't be allowed in it
    for(int i = 0; i < strlen(plaintext); i++){
        if(plaintext[i] != 32){
            if(plaintext[i] > 90 || plaintext[i] < 65){
                //if there are illegal characters, end the program prematurely
                error("CLIENT: Illegal characters in the input!");
                break;
            }
        }
    }
}

FILE* grab_file(char* path){
    //returns a file pointer for the specified path
    FILE* target_file = fopen(path, "r");

    if(target_file == NULL){
        //if there is no target file, report that the path is invalid and end the program
        error("CLIENT: Invalid file path!");
    }else{
        return target_file; //return the file pointer
    }

}

int main(int argc, char *argv[]) {
    /*
    * argv[0] == program name
    * argv[1] == plaintext file
    * args[2] == encyption key file
    * args[3] == port number
    */

    int socketFD, portNumber, charsWritten, charsRead;
    struct sockaddr_in serverAddress;
    char buffer[150000];
    char plaintext[75000];
    char key[75000];
    // Check usage & args
    if (argc < 4) { 
        fprintf(stderr,"USAGE: %s plaintext key port#\n", argv[0]); 
        exit(0); 
    } 

    // Create a socket
    socketFD = socket(AF_INET, SOCK_STREAM, 0); 
    if (socketFD < 0){
        error("CLIENT: ERROR opening socket");
    }

    // Set up the server address struct
    setupAddressStruct(&serverAddress, atoi(argv[3]));

    // Connect to server
    if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
        error("CLIENT: ERROR connecting");
    }

    FILE* plaintextFD = grab_file(argv[1]); //get the file pointer for the plaintext file

    // Clear out the buffer array
    memset(buffer, '\0', sizeof(buffer));
    // Get input from the user, trunc to buffer - 1 chars, leaving \0
    fgets(buffer, sizeof(buffer) - 1, plaintextFD);
    // Remove the trailing \n that fgets adds
    buffer[strcspn(buffer, "\n")] = '\0';

    strcpy(plaintext, buffer); //store the plaintext data from the buffer in its own variable

    fclose(plaintextFD);
    FILE* keyFD = grab_file(argv[2]); //get the file pointer for the encryption key file

    //now send the key to the server
    memset(buffer, '\0', sizeof(buffer));
    fgets(buffer, sizeof(buffer) - 1, keyFD);
    buffer[strcspn(buffer, "\n")] = '\0'; 

    strcpy(key, buffer); //store the key data from the buffer in its own variable

    fclose(keyFD);

    if(strlen(plaintext) > strlen(key)){
        //checks if the key is of an adequate length, and if not, handle the error
        error("CLIENT: Key is too short!");
    }

    check_for_bad_chars(plaintext); //check if there are any illegal characters in the plaintext

    memset(buffer, '\0', sizeof(buffer));
    //smash all of the data together in one string so it can be sent to the server
    sprintf(buffer, "%s##%s##%s", id, plaintext, key); 

    charsWritten = send(socketFD, buffer, strlen(buffer), 0); 
    if (charsWritten <= 0){
        error("CLIENT: ERROR writing to socket");
    }
    if (charsWritten < strlen(buffer)){
        //printf("CLIENT: WARNING: Not all data written to socket!\n");
    }

    // Get return message from server
    // Clear out the buffer again for reuse
    memset(buffer, '\0', sizeof(buffer));
    // Read data from the socket, leaving \0 at end
    charsRead = recv(socketFD, buffer, 150000, 0); 
    if (charsRead < 0){
        error("CLIENT: ERROR reading from socket");
    }
    fprintf(stdout, "%s\n", buffer); //output the encrypted test to STDOUT


    // Close the socket
    close(socketFD); 
    return 0;
}