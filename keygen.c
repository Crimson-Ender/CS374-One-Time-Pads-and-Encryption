/*
* MAX BAKER
* keygen.c
* March 12th, 2024
*/


#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <errno.h>

//allows us to randomly generate a character by randomly generating an index of this string of valid characters
const char* charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZ "; 

char* generate_key(int key_length){
    
    char* key = malloc(sizeof(char)*key_length); //allocate memory for key
    srand(time(NULL)); //seed the random number generator to generate a random key

    for(int i = 0; i<key_length;i++){
        int rand_idx = rand()%27; //generates a number between 0-26, the valid indicies for charset
        key[i] = charset[rand_idx]; //populates the key
    }    

    return key;
}

int main(int argc, void* argv[]){
    int key_length;

    //check if the user input a valid argument for the length of the key
    if(argv[1] == NULL || argv[1] == 0){
        //invalid input, either NULL or of length 0
        errno = 5; //error code corresponds to input/output error
        perror("Please enter a valid length that is greater than 0\n");
        return 0;
    }else{
        key_length = atoi(argv[1]); //converts the input string to an integer
    }
    char* key = generate_key(key_length); //generates the key

    printf(key);

    free(key);
    return 0;
}