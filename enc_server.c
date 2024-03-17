#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "enc_server.h"

const char* charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZ "; 

// Error function used for reporting issues
void error(const char *msg) {
  perror(msg);
  exit(1);
} 

// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in* address, int portNumber){
 
  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address)); 

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);
  // Allow a client at any address to connect to this server
  address->sin_addr.s_addr = INADDR_ANY;
}

char* encrypt_message(char* plaintext, char* key){
    char* ciphertext = malloc(sizeof(char)*strlen(plaintext));
    
    /*
    printf("DEBUG: Prior to conversion\n");
    printf("DEBUG: Plaintext == %s\n", plaintext);
    printf("DEBUG: strlen(plaintext) == %d\n", strlen(plaintext));
    printf("DEBUG: key == %s\n", key);
    */

    //convert both the plaintext and key text into numerial values using their ASCII values
    int i;
    for(i = 0; i<strlen(plaintext); i++){
        int pt_val = plaintext[i] - 65;   //ASCII value for 'A' is 65, so we subtract 65 to ensure 'A' is zero
        if(pt_val == -33){
            pt_val = 26;  //space's ASCII value is 32, so it would be -33 after subtracting 65 from it
        }
        int key_val = key[i] -65;
        if(key_val == -33){
            key_val == 26;
        }

        ciphertext[i] = charset[(pt_val + key_val)%27];
    }

    /*
    printf("DEBUG: After conversion\n");
    printf("DEBUG: Plaintext == %s\n", ciphertext);
    printf("DEBUG: strlen(plaintext) == %d\n", strlen(plaintext));
    printf("DEBUG: number of characters conversions == %d\n", i);
    printf("DEBUG: key == %s\n", key);
    printf("DEBUG: Ciphertext == %s\n", ciphertext);
    */
    return ciphertext;
}

int main(int argc, char *argv[]){
  int connectionSocket, charsRead;
  char buffer[256];
  struct sockaddr_in serverAddress, clientAddress;
  socklen_t sizeOfClientInfo = sizeof(clientAddress);

  // Check usage & args
  if (argc < 2) { 
    fprintf(stderr,"USAGE: %s port\n", argv[0]); 
    exit(1);
  } 
  
  // Create the socket that will listen for connections
  int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (listenSocket < 0) {
    error("ERROR opening socket");
  }

  // Set up the address struct for the server socket
  setupAddressStruct(&serverAddress, atoi(argv[1]));

  // Associate the socket to the port
  if (bind(listenSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0){
    error("ERROR on binding");
  }

  // Start listening for connetions. Allow up to 5 connections to queue up
  listen(listenSocket, 5); 
  
  // Accept a connection, blocking if one is not available until one connects
  while(1){
    // Accept the connection request which creates a connection socket
    connectionSocket = accept(listenSocket, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); 
    if (connectionSocket < 0){
      error("ERROR on accept");
    }

    //branch off into a child process

    printf("SERVER: Connected to client running at host %d port %d\n", 
                          ntohs(clientAddress.sin_addr.s_addr),
                          ntohs(clientAddress.sin_port));

    // Get the message from the client and display it
    memset(buffer, '\0', 256);
    // Read the client's message from the socket
    charsRead = recv(connectionSocket, buffer, 255, 0); 
    if (charsRead < 0){
      error("ERROR reading from socket");
    }

    char* plaintext[strlen(buffer)];
    strcpy(plaintext, buffer);
    printf("SERVER: I received this from the client: \"%s\"\n", plaintext);

    // Get the message from the client and display it
    memset(buffer, '\0', 256);
    // Read the client's message from the socket
    charsRead = recv(connectionSocket, buffer, 255, 0); 
    if (charsRead < 0){
      error("ERROR reading from socket");
    }

    char* key[strlen(buffer)];
    strcpy(key, buffer);
    printf("SERVER: I received this from the client: \"%s\"\n", key);

    char* ciphertext = encrypt_message(plaintext, key);
    // Send a Success message back to the client
    charsRead = send(connectionSocket, ciphertext, strlen(ciphertext), 0); 
    
    if (charsRead < 0){
      error("ERROR writing to socket");
    }
    // Close the connection socket for this client
    close(connectionSocket);
    free(ciphertext); 
  }
  // Close the listening socket
  close(listenSocket); 
  return 0;
}