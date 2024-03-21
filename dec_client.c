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

const char* id = "dec_client"; //used to identify the program to the server

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

FILE* grab_file(char* path){
    //get the file pointer for the specified path
    FILE* target_file = fopen(path, "r");

    if(target_file == NULL){
        //if there is not a valid file at that path, throw an error and exit the program
        error("CLIENT: Invalid file path!");
    }else{
        return target_file;
    }

}

int main(int argc, char *argv[]) {
    /*
    * argv[0] == program name
    * argv[1] == cipher file
    * args[2] == encyption key file
    * args[3] == port number
    */

    int socketFD, portNumber, charsWritten, charsRead;
    struct sockaddr_in serverAddress;
    char buffer[150000];
    char ciphertext[75000];
    char key[75000];
    // Check usage & args
    if (argc < 4) { 
        fprintf(stderr,"USAGE: %s ciphertext key port#\n", argv[0]); 
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

    FILE* ciphertextFD = grab_file(argv[1]); //get the file pointer for the ciphertext file

    // Clear out the buffer array
    memset(buffer, '\0', sizeof(buffer));
    // Get input from the user, trunc to buffer - 1 chars, leaving \0
    fgets(buffer, sizeof(buffer) - 1, ciphertextFD);
    // Remove the trailing \n that fgets adds
    buffer[strcspn(buffer, "\n")] = '\0';

    strcpy(ciphertext, buffer); //copy the ciphertext from the buffer into its own variable

    fclose(ciphertextFD);
    FILE* keyFD = grab_file(argv[2]); //grab the file pointer for the key file

    //now send the key to the server
    memset(buffer, '\0', sizeof(buffer));
    fgets(buffer, sizeof(buffer) - 1, keyFD);
    buffer[strcspn(buffer, "\n")] = '\0'; 

    strcpy(key, buffer); //copy the key from the buffer into its own variable

    fclose(keyFD);

    //check if the key is an adequate length for the ciphertext, otherwise, the program will end
    if(strlen(ciphertext) > strlen(key)){
        error("CLIENT: Key is too short!");
    }

    //prepares the buffer to send data to the server
    memset(buffer, '\0', sizeof(buffer));
    sprintf(buffer, "%s##%s##%s", id, ciphertext, key); //jams all the data into one string to send to the server to be parsed later

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
    fprintf(stdout, "%s\n", buffer); //output the decrypted text to stdout


    // Close the socket
    close(socketFD); 
    return 0;
}