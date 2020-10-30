#include <stdio.h>
#include <stdlib.h>
#include <time.h>
int error(char * temp);
int main(int argc, char *argv[]){
	char keyrandom;
	int klength = 0;
	srand(time(0)); //Using srand to produce random number
	//Checking the number of arguments
    int check1;
    check1 = (argc != 2)?1:-1;
    switch(check1){
        case 1:
            error("ineffective number of arguments\n");
            break;
    }
	//Storing the number of keys from argv[1] to the varaible
	//sscanf : http://ccckmit.wikidot.com/cp:sscanf
	sscanf(argv[1], "%d", &klength);
    int check2;
    check2 = (klength <= 0)?1:-1;
    switch(check2){
        case 1:
            error("ineffective number of keys\n");
            break;
    }
	int i = 0;
    while(i < klength){
		keyrandom = (char)(rand() % (90 - 64 + 1) + 64);
		// let the 27th word be the space
        int check3;
        check3 = (keyrandom == '@')?1:-1;
        switch(check3){
            case 1:
                keyrandom = ' ';
                break;
        }
		printf("%c", keyrandom);
        i++;
	}
	printf("\n");
	return 0;
}
int error(char * temp){
	fprintf(stderr, temp);
	return 1;
}

