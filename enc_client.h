#ifndef __ENC_CLIENT_H
#define __ENC_CLIENT_H

void error(const char *msg);
void setupAddressStruct(struct sockaddr_in* address, int portNumber);
FILE* grab_file(char* path);

#endif