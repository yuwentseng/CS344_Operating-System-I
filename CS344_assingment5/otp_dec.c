#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/ioctl.h>

#define NUM1 71000
#define NUM2 142000

void checkarg(int argc,char * str);
void error(const char* message);
int receive(int skt, char *buf , int size , int num);
int file_size(char* file);
int trans(int skt ,char *buf , int size , int num);

// Error function used for reporting issues
void error(const char* message){
	fprintf(stderr, "%s\n", message);
}
// Retrun the size of the file
int file_size(char* file) {
	int filelength = 0;
	//Open the file and get the length of text
	FILE* fp = fopen(file, "rb"); // use "rb" to ignore newline
    int check1;
    check1 =(fp != NULL)?1:-1;
    switch(check1){
        case 1:
            fseek(fp, 0, SEEK_END);// fseek() & ftell(): https://www.geeksforgeeks.org/fseek-in-c-with-example/
            // store the position of pointer
            filelength = ftell(fp);
            fclose(fp);
            break;
    }
	return filelength;
}
void checkarg(int argc,char * str){
    int check2;
    check2 = (argc < 4)?1:-1;
    switch(check2){
        case 1:
            fprintf(stderr, "USAGE: %s text key port\n", str);
            exit(0);
            break;
    }
}
int main(int argc, char *argv[]) {
	int socketfd, portnum;
	struct sockaddr_in serverlocation;
	struct hostent* serverowninfo;
	int sizeoftext, lengthoftext, sizeofkey;
	char textbuffer[NUM1], keybuffer[NUM1], cmbuffer[NUM2], decryptdatabuffer[NUM1];
	memset(textbuffer, '\0', sizeof(textbuffer)); //Eliminating the buffer for text
	memset(keybuffer, '\0', sizeof(keybuffer)); // Eliminating the buffer for key
	memset(cmbuffer, '\0', sizeof(cmbuffer)); // Eliminating the buffer for complete message
	memset(decryptdatabuffer, '\0', sizeof(decryptdatabuffer)); //Eliminating the buffer for decrypted data
	// Check usage and arguments
    checkarg(argc,argv[0]);
    int check2;
    check2 = (argc < 4)?1:-1;
    switch(check2){
        case 1:
            fprintf(stderr, "USAGE: %s text key port\n", argv[0]);
            exit(0);
            break;
    }
	// Set up the server location struct
	memset((char*)&serverlocation, '\0', sizeof(serverlocation)); //Eliminating the location struct
	portnum = atoi(argv[3]); // Get the port number and tranfer the string to an integer
	serverlocation.sin_family = AF_INET; // Create a network-capable socket
	serverlocation.sin_port = htons(portnum); // Store the port number
	serverowninfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
    int check3;
    check3 = (serverowninfo == NULL)?1:-1;
    switch(check3){
        case 1:
            fprintf(stderr, "CLIENT: Failure, no such host\n");
            exit(0);
            break;
    }
	memcpy((char*)&serverlocation.sin_addr.s_addr, (char*)serverowninfo->h_addr, serverowninfo->h_length); // Copy in the address
	// Get the size of text in plaintext for the buffer
    char * str2 = argv[1];
	sizeoftext = file_size(str2);
    int check4;
    check4 = (sizeoftext == 0)?1:-1;
    switch(check4){
        case 1:
            error("no existed file or no contents in the file");
            exit(1);
            break;
    }
	sizeoftext += 2; //Include the size for terminating characters

	// Get the size of key file
    char * str = argv[2];
	sizeofkey = file_size(str);
    int check5;
    check5 = (sizeofkey == 0)?1:-1;
    switch(check5){
        case 1:
            error("no existed file or no contents in the file");
            exit(1);
            break;
    }
	sizeofkey += 2; // Include the size for terminating characters

	// Check if the number of key is enough
    int check6;
    check6 = (sizeofkey+1 < sizeoftext)?1:-1;
    switch(check6){
        case 1:
            fprintf(stderr, "ERROR: key '%s' is too short\n", argv[2]);
            exit(1);
            break;
    }
	// Store the text in the plaintext into buffer
	FILE* fp = fopen(argv[1], "rb"); // Use "rb" to ignore newline
    int check7;
	if (fp == NULL) {
        check7 = 1;
    }
    else{
        check7 = 2;
    }
    switch(check7){
        case 1:
            error("plaintext does not exist\n");
            exit(1);
            break;
        case 2:
            // Move pointer to the end
            fseek(fp, 0, SEEK_END);
            // Store the position of pointer
            lengthoftext = ftell(fp);
            // Move pointer to the starting
            fseek(fp, 0, SEEK_SET);
            // Read the file
            fread(textbuffer, lengthoftext, 1, fp);
            // Replace newline & check the character
            int i = 0, ascii;
            while(i < lengthoftext){
                ascii = (int)textbuffer[i];
                // Replace newline with '*'
                int check8;
                if(textbuffer[i] == '\n'){
                    check8 = 1;
                }
                else{
                    check8 = 2;
                }
                switch(check8){
                    case 1:
                        textbuffer[i] = '*';
                        strcat(textbuffer, "*");
                        break;
                    case 2:
                        if(ascii < 65 || ascii > 90) {
                            if(ascii != 32){
                                if(ascii != 64){
                                    error("Error: invalid character in plaintext (decrypt)\n");
                                    fclose(fp);
                                    exit(1);
                                }
                            }
                        }
                }i++;
            }fclose(fp);
    }
	// Set up the socket
	socketfd = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
    int check9;
    check9 = (socketfd < 0)?1:-1;
    switch(check9){
        case 1:
            error("CLIENT: Fail to open socket");
            exit(1);
            break;
    }
	// Connect to server
    int check10;
    check10 = (connect(socketfd, (struct sockaddr*)&serverlocation, sizeof(serverlocation)) < 0)?1:-1;
    switch(check10){
        case 1:
            error("CLIENT: Fail to connect to server");
            exit(1);
            break;
    }
	// Store the key into buffer
	FILE* fp_2 = fopen(argv[2], "rb"); // Use "rb" to ignore newline
    int check11;
	if (fp_2 == NULL) {
        check11 = 1;
    }
    else{
        check11 = 2;
    }
    switch(check11){
        case 1:
            error("key does not exist\n");
            exit(1);
            break;
        case 2:
            // Move pointer to the starting
            fseek(fp_2, 0, SEEK_SET);
            // Read the file
            fread(keybuffer, sizeoftext-3, 1, fp_2);
            // Replace newline & check the character
            int i = 0, ascii_2 = 0, is_replace = 0;
            while(i < sizeoftext-3){
                ascii_2 = (int)keybuffer[i];
                // Replace newline with '@'
                int check12;
                if (keybuffer[i] == '\n') {
                    check12 = 1;
                }
                else{
                    check12 = 2;
                }
                switch(check12){
                    case 1:
                        keybuffer[i] = '@';
                        strcat(keybuffer, "@");
                        is_replace = 1;
                        break;
                    case 2:
                        if(ascii_2 < 65 || ascii_2 > 90){
                            if(ascii_2 != 32){
                                if(ascii_2 != 64){
                                    error("invalid character in key\n");
                                    fclose(fp_2);
                                }
                            }
                        }
                        break;
                }i++;
            }
            // if no '\n' in the buffer
            int check13;
            check13 = (is_replace == 0)?1:-1;
            switch(check13){
                case 1:
                    keybuffer[sizeoftext-3] = '@';
                    strcat(keybuffer,"@");
                    break;
            }
            fclose(fp_2);
	}
	// Store the content of text and key into complete buffer
	strcat(cmbuffer, textbuffer);
	strcat(cmbuffer, keybuffer);
	// Send data to server to for decryting
	int writtendata;
	int index1 = 0;
	int bufferweight = sizeof(cmbuffer) - 1;
	// Write data until cmbuffer is filled
	while(index1 < bufferweight){
		writtendata = trans(socketfd, cmbuffer+index1, bufferweight, 0);
        int check14;
        check14 = (writtendata < 0)?1:-1;
        switch(check14){
            case 1:
                error("CLIENT: Fail to write to socket");
                exit(1);
                break;
        }
		index1 += writtendata;
		bufferweight -= writtendata;
    }
	// Read the decrypted data from server
	int readdata;
	int index2 = 0;
	int bufferweight2 = sizeof(decryptdatabuffer) - 1;
	// Receive data until decryptdatabuffer is filled
	while(index2 < sizeof(decryptdatabuffer) - 1){
		readdata = receive(socketfd, decryptdatabuffer+index2, bufferweight2, 0);
        int check15;
        check15 = (readdata < 0)?1:-1;
        switch(check15){
            case 1:
                error("CLIENT: Fail to read from socket");
                exit(1);
                break;
        }
		index2 += readdata;
		bufferweight2 -= readdata;
    }
	// Change the "@@" at the end of decryptdatabuffer into termination symbol
	int termposition = strstr(decryptdatabuffer, "@@") - decryptdatabuffer;
	decryptdatabuffer[termposition] = '\0';
	// Print out the data from decryption
	printf("%s\n", decryptdatabuffer);
	close(socketfd);
	return 0;
}
int trans(int skt ,char *buf , int size , int num){
    return send(skt, buf, size, num);
}
int receive(int skt, char *buf , int size , int num){
    return recv(skt, buf, size, num);
}
