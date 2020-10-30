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
#define FALSE 0

#define NUM1 142000
#define NUM2 71000

int stringtoi(char * str);
void error(const char* message);
void checkargc(int argc,char *argv[]);
void dsperror(char * str);
int transfer(int skt,char * buf , int weit , int num);

// Error function used for reporting issues
void error(const char* message) {
	fprintf(stderr, "%s\n", message);
}
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
	struct sockaddr_in servlocation, clientlocation;
	char cmbuffer[NUM1], textbuffer[NUM2], keybuffer[NUM2], decryptdabuffer[NUM2];
	memset(cmbuffer, '\0', sizeof(cmbuffer)); //Eliminating the buffer for complete message
	memset(textbuffer, '\0', sizeof(textbuffer)); //Eliminating the buffer for text
	memset(keybuffer, '\0', sizeof(keybuffer)); //Eliminating the buffer for key
	memset(decryptdabuffer, '\0', sizeof(decryptdabuffer)); //Eliminating the buffer for decrypted data
	// Check usage and arguments
	checkargc(argc,&argv[0]);

	// Set up the location struct for this process
	memset((char *)&servlocation, '\0', sizeof(servlocation)); //Eliminating the location struct
	portnum = stringtoi(argv[1]); // Get the port number and transfer the string to integer
	servlocation.sin_family = AF_INET; // Create a network-capable socket
	servlocation.sin_port = htons(portnum); // Store the port number
	servlocation.sin_addr.s_addr = INADDR_ANY; // Any location is allowed for linking to this process
	// Set up the socket
	receivesocketfd = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
    int check5;
    check5 = (receivesocketfd < 0)?1:-1;
    switch(check5){
        case 1:
            dsperror("Fail to open socket");
    }
	// Enable the socket to start receiving
    int check6;
    check6 = (bind(receivesocketfd, (struct sockaddr *)&servlocation, sizeof(servlocation)) < 0)?1:-1;
    switch(check6){
        case 1:
            dsperror("Fail to bind");

    }
	listen(receivesocketfd, 5); // Flip the socket on and it can receive up to 5 linkss now
	// Keep implementing the server for linking
	while(TRUE){
		// Accept a connection, blocking if one is unavailable until one connects
		weightofclientinfo = sizeof(clientlocation); //Get the size of the address for the client that will link
		setlinkfd = accept(receivesocketfd, (struct sockaddr *)&clientlocation, &weightofclientinfo);
        int check2;
        check2 = (setlinkfd < 0)?1:-1;
        switch(check2){
            case 1:
                dsperror("Fail to accept");
    
        }
		//Get the message from the client
		int readdata;
		int index1 = 0;
		int bufferweight = sizeof(cmbuffer) - 1; //if use -- is still right , use sizeof(cmbuffer)--;
		// receive data until cmbuffer is filled
		while(index1 < bufferweight){
			readdata = recv(setlinkfd, cmbuffer+index1, bufferweight, 0);
            int check3;
            check3 = (readdata < 0)?1:-1;
            switch(check3){
                case 1:
                    dsperror("SERVER: Fail to read from socket");
                 
            }
			index1 += readdata;
			bufferweight -= readdata;
        }
		// Handling complete message from client into text and key
		char* endloc = strstr(cmbuffer, "**");
		int termlength = strlen("**");
		// Copy only text from cmbuffer to textbuffer
		strncpy(textbuffer, cmbuffer, endloc-cmbuffer);
		// Fill the key buffer
		sprintf(keybuffer, "%s", endloc+termlength);
		// replace "@@" from the end of key to '\0'
		int termposition = strstr(keybuffer, "@@") - keybuffer;
		keybuffer[termposition] = '\0';
		// Decrypt the text by the key (OTP)
		int i = 0;
        while(i < strlen(textbuffer)){
			int text_i = (int)textbuffer[i]; //if remove int still right , remove it
			int key_i = (int)keybuffer[i];//if remove int still right , remove it
			int decrypt_i;
            switch(text_i){ // 'text', replace space into 26th character
                case 32:
                    text_i = 26;
                    break;
                default: // 'text', if not space, minus 65 ('A')
                    text_i = text_i- 65;
                    break;
            }
            switch(key_i){
                case 32: // 'key', replace space into 26th character
                    key_i = 26;
                    break;
                default: // 'key', if not space, minus 65 ('A')
                    key_i =key_i- 65;
                    break;
            }
            // 'decryption' by minusing text by key
            decrypt_i = text_i - key_i;
            // 'decreption', make sure the range is from 0 to 26
            int check7;
            check7 = (decrypt_i < 0)?1:-1;//this part you use switch case
            switch(check7){
                case 1:
                    decrypt_i =decrypt_i+ 27;
                    break;
            }
            // 'decryption', transform the 27th into space
            int check8;
            if(decrypt_i == 26){
                check8 = 1;
            }
            else{
                check8 = 2;
            }
            switch(check8){
                case 1:
                    decrypt_i = 32; // 'decryption', transform back to 'A' to 'Z'
                    break;
                case 2:
                    decrypt_i =decrypt_i+ 65; // 'decryption', transform back to char from int and store into d_buffer
                    break;
            }
            decryptdabuffer[i] = (char)decrypt_i; //if remove char still right , remove it
            i++;
		}
		// Add termination symbol
		strcat(decryptdabuffer, "@@");
		// Send decryted data back to client
		int datawritten;
		int index2 = 0;
		int bufferweight2 = sizeof(decryptdabuffer) - 1; // if use -- still right , use --
		while(index2 < bufferweight2){
			
			datawritten = transfer(setlinkfd, decryptdabuffer+index2, bufferweight2, 0);
            int check4;
            check4 = (datawritten < 0)?1:-1;
            switch(check4){
                case 1:
                    dsperror("SERVER: Fail to write to socket");
                 
            }
			index2 =index2 + datawritten;
			bufferweight2  =bufferweight2 - datawritten;
        }
		close(setlinkfd); // Close the existing socket which is connected to the client
	}
	close(receivesocketfd); // Close the receiving socket
	return 0;
}
int transfer(int skt,char * buf , int weit , int num){
    return send(skt, buf, weit, num);
}
int stringtoi(char * str){
	int rnt = atoi(str);
	return rnt;
}
