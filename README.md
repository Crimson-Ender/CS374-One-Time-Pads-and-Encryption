# Assignment 5: One-Time Pads and Encryption

This program consists of five distinct programs that operate like a one-time pad encryption system.

Keygen will generate a psuedo-random encryption key.

enc_client and enc_server interact with each other to handle the process of decryption. The user will use enc_client to send a plaintext file and an encryption key to enc_server. enc_server will then use the encryption to encrypt the plaintext and send the encrypted ciphertext back to the client.

dec_client and dec_server operate basically the same as their encryption counterparts, however dec_client takes the encrypted ciphertext and the encryption key and sends it to the server so the server can decrypt it and send back the original plaintext back to the client.

enc_server and enc_client will both continually in the background as daemons.

The Makefile will compile all of the programs when you use the command "make" in the directory containing all of the program's file.