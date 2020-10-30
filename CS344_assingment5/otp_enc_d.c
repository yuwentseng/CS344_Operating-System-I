//Tseng, Yu-Wen
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/ioctl.h>

#define TRUE 1

#define NUM1 142000
#define NUM2 71000

void error(const char* message);
void checkargc(int argc,char *argv[]);
void dsperror(char * str);
int transfer(int skt,char * buf , int weit , int num);
int stringtoi(char * str);

// Error function used for reporting issues
void error(const char* message){
	fprintf(stderr, "%s\n", message);
}
// Check usage and arguments
void checkargc(int argc,char *argv[]){
    int check1;
    check1 = (argc < 2)?1:-1;
    switch(check1){
        case 1:
            fprintf(stderr, "USAGE: %s port\n", argv[0]);
            exit(1);
            break;
    }
}
void dsperror(char * str){
    error(str);
    exit(1);
}
int main(int argc, char *argv[]) {
	int receivesocketfd, setlinkfd, portnum;
	socklen_t weightofclientinfo;
	struct sockaddr_in serlocation, clientlocation;
	char cmbuffer[NUM1], textbuffer[NUM2], keybuffer[NUM2], encryptdatabuffer[NUM2];
	memset(cmbuffer, '\0', sizeof(cmbuffer)); // Eliminating the buffer for complete message
	memset(textbuffer, '\0', sizeof(textbuffer)); // Eliminating the buffer for text
	memset(keybuffer, '\0', sizeof(keybuffer)); // Eliminating the buffer for key
	memset(encryptdatabuffer, '\0', sizeof(encryptdatabuffer)); // Eliminating the buffer for encrypted data
	// Set up the location struct for this process
	memset((char *)&serlocation, '\0', sizeof(serlocation)); // Eliminating the address struct
	portnum = stringtoi(argv[1]); // Get the port number and tranfer hte string to an integer
	serlocation.sin_family = AF_INET; // Create a network-capable socket
	serlocation.sin_port = htons(portnum); // Store the port number
	serlocation.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process
	// Set up the socket
	receivesocketfd = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
    int check2;
    check2 = (receivesocketfd < 0)?1:-1;
    switch(check2){
        case 1:
            dsperror("Fail to open the socket");
    }
	// Enable the socket to start receiving
    int check3;
    check3 = (bind(receivesocketfd, (struct sockaddr *)&serlocation, sizeof(serlocation)) < 0)?1:-1;
    switch(check3){
        case 1:
            dsperror("Fail to bind");
    }
	listen(receivesocketfd, 5); // Flip the socket on and it can receive up to 5 connections now
	// Keep implementing the server for linking
	while(TRUE) {
		// Accept a connection, blocking if one is unavailable until one connects
		weightofclientinfo = sizeof(clientlocation); // Get the size of the address for the client that will connect
		setlinkfd = accept(receivesocketfd, (struct sockaddr *)&clientlocation, &weightofclientinfo);
        int check4;
        check4 = (setlinkfd < 0)?1:-1;
        switch(check4){
            case 1:
                dsperror("ERROR on accept");
        }
		// Get the message from the client
		int readdata;
		int index1 = 0;
		int bufferweight = sizeof(cmbuffer) - 1;
		// Receive data until cmbuffer is filled
		while(index1 < bufferweight) {
			readdata = recv(setlinkfd, cmbuffer+index1, bufferweight, 0);
            int check5;
            check5 = (readdata < 0)?1:-1;
            switch(check5){
                case 1:
                    dsperror("SERVER: Fail to read from socket");
                   
            }
			index1 += readdata;
			bufferweight -= readdata;
		}
		// Handling complete message from client into text & key
		char* endloc = strstr(cmbuffer, "**");
		int termlength = strlen("**");
		// Copy only text from cmbuffer to textbuffer
		strncpy(textbuffer, cmbuffer, endloc-cmbuffer);
		// Fill the key buffer
		sprintf(keybuffer, "%s", endloc+termlength);
		// Replace "@@" from the end of key to '\0'
		int termposition = strstr(keybuffer, "@@") - keybuffer;
		keybuffer[termposition] = '\0';
		// Encrypt the text by the key (OTP)
		int i = 0;
        while(i < strlen(textbuffer)){
			int text_i = (int)textbuffer[i];
			int key_i = (int)keybuffer[i];
			int encrypt_i;
			// 'text', replace space into 26th character
            int check6;
            if(text_i == 32){
                check6 = 1;
            }
            else{
                check6 = 2;
            }
            switch(check6){
                case 1:
                   text_i = 26;
                    break;
                case 2:
                   text_i -= 65;
                    break;
            }
			// 'key', replace space into 26th character
            int check7;
            if(key_i == 32){
                check7 = 1;
            }
            else{
                check7 = 2;
            }
            switch(check7){
                case 1:
                    key_i = 26;
                    break;
                case 2:
                    key_i -= 65;
                    break;
            }
			// 'encryption' by adding text & key
			encrypt_i = text_i + key_i;
			encrypt_i = encrypt_i % 27;
			// 'encryption', transform the 27th into space
            int check8;
            if(encrypt_i == 26){
                check8 = 1;
            }
            else{
                check8 = 2;
            }
            switch(check8){
                case 1:
                    encrypt_i = 32;
                    break;
                case 2:
                    encrypt_i += 65;
                    break;
            }
			encryptdatabuffer[i] = (char)encrypt_i;
            i++;
		}
		// add termination symbol
		strcat(encryptdatabuffer, "@@");
		// Send encryted data back to client
		int datawritten;
		int index2 = 0;
		int bufferweight2 = sizeof(encryptdatabuffer) - 1;
		while(index2 < sizeof(encryptdatabuffer) - 1) {
			datawritten = transfer(setlinkfd, encryptdatabuffer+index2, bufferweight2, 0);
            int check9;
            check9 = (datawritten < 0)?1:-1;
            switch(check9){
                case 1:
                    dsperror("SERVER: Fail to write to socket");
			}
			index2 += datawritten;
			bufferweight2 -= datawritten;
		}
		close(setlinkfd); // Close the existing socket which is connected to the client
	}
	close(receivesocketfd); // Close the listening socket
	return 0;
}
int transfer(int skt,char * buf , int weit , int num){
   return send(skt, buf, weit, num);
}
int stringtoi(char * str){
    int rnt = atoi(str);
    return rnt;
}
