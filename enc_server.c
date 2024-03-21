/*
* Max Baker
* enc_server.c 
* March 20th, 2024
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>

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

int get_ASCII_val(int pt_val, int key_val){
  //calculates the ASCII value
  int ASCII_val = pt_val + key_val;
  ASCII_val = ASCII_val % 27;
  ASCII_val = ASCII_val+65;
  return ASCII_val;
}

char* encrypt_message(char* plaintext, char* key){
    char* ciphertext = malloc(sizeof(char)*strlen(plaintext));

    fflush(stdout);
    
    int i;
    for(i = 0; i < strlen(plaintext); i++){
        int pt_val = plaintext[i] - 65;   //ASCII value for 'A' is 65, so we subtract 65 to ensure 'A' is zero
        if(pt_val == -33){
            pt_val = 26;  //space's ASCII value is 32, so it would be -33 after subtracting 65 from it
        }
        int key_val = key[i] -65;
        if(key_val == -33){
            key_val = 26;
        }

        if(((pt_val+key_val)%27) == 26){
          ciphertext[i] = 32;
        }else{
          ciphertext[i] = get_ASCII_val(pt_val, key_val);
        }
    }
    return ciphertext;
}
 
int main(int argc, char *argv[]){
  int connectionSocket, charsRead;
  char buffer[150000];
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
    pid_t child_pid = fork();
    if(child_pid == 0){

    //printf("SERVER: Connected to client running at host %d port %d\n", 
                          //ntohs(clientAddress.sin_addr.s_addr),
                          //ntohs(clientAddress.sin_port));

    // Get the message from the client and display it
    memset(buffer, '\0', 150000);
    // Read the client's message from the socket
    charsRead = recv(connectionSocket, buffer, 150000, 0); 
    if (charsRead < 0){
      error("ERROR reading from socket");
    }

    //printf("strlen(buffer) == %d\n",strlen(buffer));

    //allocates the memory so the string can be parsed properly (i tried doing this without malloc but it got fucky)
    char* str_to_parse = malloc(sizeof(char)*15000);
    char* id = malloc(sizeof(char)*150);
    char* plaintext = malloc(sizeof(char)*75000);
    char* key = malloc(sizeof(char)*75000);
    strcpy(str_to_parse, buffer);
    char* save_string = str_to_parse;

    //parse the buffer into usable strings
    id = strtok_r(str_to_parse, "##", &save_string);
    plaintext = strtok_r(NULL, "##", &save_string);
    //printf("strlen(plaintext) == %d\n", strlen(plaintext));
    key = strtok_r(NULL, "##", &save_string);

    //check if the right program is connected
    if(strstr(id, "dec_client")!=NULL){
      close(connectionSocket);
      error("ERROR invalid client connected");
    }

    char* ciphertext = encrypt_message(plaintext, key); //encrypt the message using the algorithm in encrypt_message()

    //clear out the buffer to send data back to the client
    memset(buffer, '\0', 150000);
    strcpy(buffer, ciphertext);
    fflush(stdout);

    //send data back to the client
    charsRead = send(connectionSocket, buffer, strlen(buffer), 0); 
    if (charsRead < 0){
      error("ERROR writing to socket");
    }

    //end the child process
    free(ciphertext);
    exit(0);
    break;
    }
    wait(NULL);
    close(connectionSocket);
  }
  // Close the listening socket
  close(listenSocket); 
  return 0;
}