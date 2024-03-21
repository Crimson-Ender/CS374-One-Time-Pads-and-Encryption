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

int get_ASCII_val(int ct_val, int key_val){
//handles the ASCII math part of decryption and returns the ASCII value of the plaintext character
  int ASCII_val = ct_val - key_val;
  if(ASCII_val < 0){
    ASCII_val = ASCII_val + 27;
  }
  ASCII_val = ASCII_val % 27;
  ASCII_val = ASCII_val+65;
  return ASCII_val;
}

char* decrypt_message(char* ciphertext, char* key){
    char* plaintext = malloc(sizeof(char)*strlen(ciphertext)); //allocate memory for the plaintext
    
    int i;
    for(i = 0; i < strlen(ciphertext); i++){
        int ct_val = ciphertext[i] - 65;   //ASCII value for 'A' is 65, so we subtract 65 to ensure 'A' is zero
        if(ct_val == -33){
            ct_val = 26;  //space's ASCII value is 32, so it would be -33 after subtracting 65 from it
        }
        int key_val = key[i] -65;
        if(key_val == -33){
            key_val = 26;
        }

        //ciphertext[i] = charset[(pt_val + key_val)%27];
        if(ct_val - key_val < 0 && ((ct_val - key_val + 27)%27)==26 ){
          plaintext[i] = 32;
        }else if(ct_val - key_val >= 0 && ((ct_val - key_val)%27)==26){
          plaintext[i] = 32;
        }else{
            plaintext[i] = get_ASCII_val(ct_val, key_val);
        }
    }
    return plaintext;
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

    //allocate the memory for parsing the data
    char* str_to_parse = malloc(sizeof(char)*15000);
    char* id = malloc(sizeof(char)*150);
    char* ciphertext = malloc(sizeof(char)*75000);
    char* key = malloc(sizeof(char)*75000);
    strcpy(str_to_parse, buffer);
    char* save_string = str_to_parse;

    //parse out the string that the server received into usable pieces
    id = strtok_r(str_to_parse, "##", &save_string);
    ciphertext = strtok_r(NULL, "##", &save_string);
    key = strtok_r(NULL, "##", &save_string);

    //check if the right program is connected to the server
    if(strstr(id, "enc_client")!=NULL){
        //if the wrong program is connected, throw an error and end the program
      close(connectionSocket);
      error("ERROR invalid client connected");
    }
    
    char* plaintext = decrypt_message(ciphertext, key); //decrypt the message and return plaintext

    //clear out the buffer to prepare to send data back to the client
    memset(buffer, '\0', 150000);
    strcpy(buffer, plaintext);
    fflush(stdout);

    //send data back to the client
    charsRead = send(connectionSocket, buffer, strlen(buffer), 0); 
    if (charsRead < 0){
      error("ERROR writing to socket");
    }
    free(plaintext);
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