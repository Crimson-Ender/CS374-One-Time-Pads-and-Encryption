#ifndef __ENC_SERVER_H
#define __ENC_SERVER_H

void error(const char *msg);
void setupAddressStruct(struct sockaddr_in* address, int portNumber);
char* encrypt_message(char* plaintext, char* key);

#endif