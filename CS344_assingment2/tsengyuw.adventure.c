//*****Assignment 2*****
//Course: CS344
//Name: Yu-Wen, Tseng

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <pthread.h>
// debugging function
pthread_mutex_t mexclusive; //mutual_exclusive
//struct of each room
struct Chamber{
    char  room_name[100];
    int num_connect;
    struct Chamber* connect[6];
    int room_type;
};
void* getTime(){
    pthread_mutex_lock(&mexclusive);    // Will be blocked until the mutex is unlocked by the time input in Prompt()
    FILE* myDoc;    //I referenced a few stack overflow threads for this function: goo.gl/g69qLl and goo.gl/jU2QoT
    myDoc = fopen("currentTime.txt", "w+"); //Create and write to the file
    char bumper[100];
    struct tm *tmp;
    
    time_t cur = time (NULL);
    tmp = localtime (&cur);
    strftime (bumper, sizeof(bumper), "%I:%M%p, %A, %B %d, %Y", tmp);
    printf("%s\n", bumper);
    fputs(bumper, myDoc);
    fclose(myDoc);
    pthread_mutex_unlock(&mexclusive);
}
//I used a couple sources as a refernce for this function: https://computing.llnl.gov/tutorials/pthreads/#Mutexes
void threading(){
    pthread_t threaded;//http://www.yolinux.com/TUTORIALS/LinuxTutorialPosixThreads.html
    pthread_mutex_init(&mexclusive, NULL);
    pthread_mutex_lock(&mexclusive);
    
    int tidp = pthread_create(&threaded, NULL, getTime, NULL);
    pthread_mutex_unlock(&mexclusive); // Have the main thread unlock the mutex for the secondary thread to lock
    pthread_mutex_destroy(&mexclusive);
    usleep(100);    // Wait 100 microseconds and lock again
}
// read information from the room
void roomInfo(struct Chamber *room, FILE *input, char **array){
    char strings[128];
    char connection[128];
    int count;
    char room_type[128];
    // get Room name
    fseek(input,0,SEEK_SET);
    while (fgets(strings, 128, input) != NULL){
        //show the room name from each input
        if (sscanf(strings, "ROOM NAME: %[^\n]", room->room_name) == 1){
            break;
        }
}
    // get rooms and count the number of the links
    fseek(input,0,SEEK_SET);
    count = 0;
    room->num_connect = 0;
    while (fgets(strings, 128, input) != NULL){
        if (sscanf(strings, "CONNECTION %*d: %[^\n]", connection) == 1){
            strcpy(array[count], connection);
            room->num_connect+=1;
            count+=1;
        }
    }
    //get room type
    fseek(input,0,SEEK_SET);
    while (fgets(strings, 128, input) != NULL){
        // -1 = start; 0 = middle; 1 = end
        if (sscanf(strings, "ROOM TYPE: %[^\n]", room_type) == 1){
            if (strcmp(room_type, "START_ROOM") == 0){
                room->room_type = -1;
            }
            else if (strcmp(room_type, "END_ROOM") == 0){
                room->room_type = 1;
            }
            else if(strcmp(room_type, "MID_ROOM") == 0){
                room->room_type = 0;
            }
        }
    }
}
void writeTime(){
    FILE* myDoc;
    myDoc = fopen("currentTime.txt", "r");  //Read from the file
    char bumper[100];
    size_t length = 0;
    ssize_t read;
	
    fgets(bumper, 100, myDoc);  //Else read it into bumper and print the time string
    printf("\n%s\n", bumper);
    fclose(myDoc);
}
// initialize the room and copy the given room name to the room_name
struct Chamber *initializeRoom(char *rname){
    struct Chamber *store = malloc(200);
    strcpy(store->room_name, rname);
    //set the current number of connections to 0 and room type -> MID_ROOM
    store->num_connect = 0;
    store->room_type = 0;
    return store;
}
// the main funtion for playing the adventrue game.
void playgame(char *catalog, char *first, char *last){
	int i;
	char *route[100];
	// need to malloc each element for later
    i = 0;
    while(i<100){
		route[i] = (char *) malloc(64);
        i+=1;
	}
	int steps = 0; //Need to count the number of steps user took
	char *cur = malloc(64 * 32);
	char *curFile =  malloc(64 * 32);
	char *latter = malloc(64 * 32);
	char *latterFile =  malloc(64 * 32);
	struct Chamber *store = initializeRoom(first);
	//	for saving Room connections as array of char arrays
	char *tempConnections[7];
	// malloc each element for roominfo()
    i = 0;
    while(i<7){
		tempConnections[i] = malloc(sizeof(char *));
        i+=1;
	}
	FILE *doc;
	printf("WELCOME TO ADVENTURE!!\n\n");
	// start game at start file
	strcpy(cur, first);
	// read in files of Rooms
	while (strcmp(cur, last) != 0){
		sprintf(curFile, "%s/%s", catalog, cur);
		doc = fopen(curFile, "r");  // open the current Room file
		if (doc == NULL){
			fprintf(stderr, "Opening file wrong\n");
		}
		roomInfo(store, doc, tempConnections);  // read current Room info
		fclose(doc);
            while (doc == NULL || effectiveStep(tempConnections, store->num_connect, latter) == 0){
                // print out current Room info
                printf("CURRENT LOCATION: %s\n", store->room_name);
                printf("POSSIBLE CONNECTIONS: ");
                i = 0;
            while(i < store->num_connect){
				printf("%s", tempConnections[i]);
				if (i < store->num_connect - 1){
					printf(", ");
				}
				else{
					printf(".\n\n");
				}
                i++;
			}
			printf("WHERE TO? >");
			scanf("%s", latter);    //Get the user input for where they want to go
			printf("\n");
			// open next file with the next room's name
			sprintf(latterFile, "%s/%s", catalog, latter);
			doc = fopen(latterFile, "r");
			if(strcmp(latter, "time") == 0){  //Check if the user entered time. If so, call our time functions
      			threading();
      			writeTime();
      			printf("\n\n");
    		}
            //if there is no name in the file or list of the room's name, then jump up from the current room
			else if (doc == NULL){
				printf("HUH? I DO NOT UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
			}
            else if(effectiveStep(tempConnections, store->num_connect, latter) == 0){
                printf("HUH? I DO NOT UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
            }
		}
		while (doc == NULL || effectiveStep(tempConnections, store->num_connect, latter) == 0);
		fclose(doc);
		// copy entered name to current
		strcpy(cur, latter);
		// add entered name to route
		strcpy(route[steps], latter);
		steps++;
	}
	// end room is reached
	printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");//Tell user they won, how many steps they took, and the route they walked
	printf("YOU TOOK %d STEP", steps);
	if (steps > 1){
		printf("S");
	}
	printf(". YOUR PATH TO VICTORY WAS:\n");
    i = 0;
    while(i < steps){
		printf("%s\n", route[i]);
        i+=1;
	}
	// free dynamically allocated memory
    i = 0;
    while(i<7){
		free(tempConnections[i]);
        i+=1;
	}
    i = 0;
    while(i<100){
		free(route[i]);
        i++;
	}
	free(cur);
	free(curFile);
	free(latter);
	free(latterFile);
	free(store);
}
void createDir(char **argv, char *catalog){
	snprintf(catalog, 64, "tsengyuw.rooms.%d", getpid());
	int mkdirResult = mkdir(catalog, 0700);
}
//create two rooms together
void connectRooms(struct Chamber *froom, struct Chamber *sroom){
    //check if the connection is mutual, and increment the number of connection for each room
    if (froom->num_connect < 6){
        if(sroom->num_connect < 6){
        froom->connect[froom->num_connect] = sroom; // assure the room x connects to y, then y also connects to x
        froom->num_connect++;
        sroom->connect[sroom->num_connect] = froom;
        sroom->num_connect++;
        }
    }
}
//create the links between the rooms in a given array
void CreateLinks(struct Chamber **roomList){
    int i, overallLinks, randomRoom, matchName, linksExists;
    i = 0;
    while(i < 7){
        overallLinks = rand() % 4 + 3; // set total number of links
        while (roomList[i]->num_connect < overallLinks){
            randomRoom = rand() % 7;
            matchName = strcmp(roomList[i]->room_name, roomList[randomRoom]->room_name);
            linksExists = ConnectionAlreadyExist(roomList[i], roomList[randomRoom]);
            //check if there is no existing links and no other links to connect to itself
            if (matchName != 0){
                if(linksExists != 1){
                    connectRooms(roomList[i], roomList[randomRoom]);
                }
            }
        }i++;
    }
}
//Get the random array of room
void GetRandomRoom(struct Chamber *room, int length){
    int i, randNum;
    i = 0;
    while(i < length){
        //make a random number from 0 to 6, for the use to select random room
        randNum = rand() % length;
        struct Chamber *store = room->connect[i];
        room->connect[i] = room->connect[randNum];
        room->connect[randNum] = store;
        i++;
    }
}
//Random an array of char
void GetRandomChar(char **array, int length){
    int i, randNum;
    i = 0;
    while(i < length){
        randNum = rand() % length;
        char *store = array[i];
        array[i] = array[randNum]; //char arrays is given by random arrays
        array[randNum] = store;
        i++;
    }
}
//create a solution for the outcomes, which includes the first room for START_ROOM, the other room for END_ROOM
void createoutcome(struct Chamber **roomList){
    int i;
    int OutcomeRoomCount = rand() % 6 + 2;    // length = 2 to 7 Rooms
    // -1 = start; 0 = middle; 1 = end
    roomList[0]->room_type = -1;
    roomList[OutcomeRoomCount - 1]->room_type = 1;
    i = 0;
    while(i < OutcomeRoomCount - 1){
        if (!ConnectionAlreadyExist(roomList[i], roomList[i + 1])){
            connectRooms(roomList[i], roomList[i + 1]);
        }i++;
    }
}
// Free memory
void freeRoom(struct Chamber *room){
    int store = 0;
    store = room!=0?1:0;
    if(store == 1){
        free(room);
    }
}
//create 10 room's name and random all the room's name.
//catalog is generated by createDir()
void createGame(char *catalog, char *first, char *last){
    int i, j;
    // list of Room's names
    char *RoomNames[10] = {
        "akira",
        "aristotle",
        "bart",
        "benjamin",
        "homer",
        "jasper",
        "lisa",
        "marge",
        "maggie",
        "mary"
    };
    GetRandomChar(RoomNames, 10);   //randomly choose the room to each games
    struct Chamber *ChoosedRooms[7];    // choose the 7 rooms and copy to new array
    i = 0;
    while(i < 7){
        ChoosedRooms[i] = initializeRoom(RoomNames[i]);
        i++;
    }
    createoutcome(ChoosedRooms); // generate outcome from the random rooms
    CreateLinks(ChoosedRooms);  // create links to the choosedrooms
    i = 0;
    while(i<7){
        GetRandomRoom(ChoosedRooms[i], ChoosedRooms[i]->num_connect);
        i++;
    }
    // create files
    char *fileName = malloc(100 * sizeof(char));
    i = 0;
    while(i<7){
        snprintf(fileName, 64, "%s/%s", catalog, ChoosedRooms[i]->room_name);
        FILE *fp;
        fp = fopen(fileName, "w");
        if (fp == NULL){
            fprintf(stderr, "Opening file wrong\n");
        }
        else{
            // print room name to the files
            fprintf(fp, "ROOM NAME: %s\n", ChoosedRooms[i]->room_name);
            j = 0;
            while(j < ChoosedRooms[i]->num_connect){
                fprintf(fp, "CONNECTION %d: %s\n", j + 1,
                        ChoosedRooms[i]->connect[j]->room_name);
                j++;
            }
            fprintf(fp, "ROOM TYPE: ");
            // determine the room tyoe
            // -1 = start; 0 = middle; 1 = end
            int store;
            store = ChoosedRooms[i]->room_type;
            switch(store){
                case -1:
                    fprintf(fp, "START_ROOM\n");
                    strcpy(first, ChoosedRooms[i]->room_name);
                    break;
                case 1:
                    fprintf(fp, "END_ROOM\n");
                    strcpy(last, ChoosedRooms[i]->room_name);
                    break;
                default :
                    fprintf(fp, "MID_ROOM\n");
            }
        }
        fclose(fp);
        i++;
    }
    // free Rooms in array of Rooms
    i = 0;
    while(i<7){
        freeRoom(ChoosedRooms[i]);
        i++;
    }
    free(fileName);
}
//Returns 1 if a connection between two rooms are already exists, 0 otherwise
int ConnectionAlreadyExist(struct Chamber *froom, struct Chamber *sroom){
    int i;
    i = 0;
    while(i < froom->num_connect){
        if (froom->connect[i] == sroom){
            return 1;
        }i++;
    }
    return 0;
}
// check if the name is in a list of the effective names
int effectiveStep(char **array, int length, char *room_name){
	int i;
    i = 0;
    while(i < length){
        if (strcmp(room_name, array[i]) == 0){
			return 1;
		}i++;
	}
	return 0;
}
//Prints out the information for a supplied room.
void printRoom(struct Chamber *room){
	int i;
	printf("ROOM NAME: %s\n", room->room_name);
    i = 0;
    while(i < room->num_connect){
        printf("CONNECTION %d: %s\n", i + 1, room->connect[i]->room_name);
        i++;
	}
	printf("ROOM TYPE: ");
    switch(room->room_type){
        case -1:
            printf("START_ROOM\n");
            break;
        case 1:
            printf("END_ROOM\n");
            break;
        default :
            printf("MID_ROOM\n");
            
    }
}
int main(int argc, char **argv){
    // generate the random number
    srand(time(NULL));
    // create directories
    char *dirName = malloc(128*sizeof(char));
    createDir(argv, dirName);
    char *first = malloc(128*sizeof(char));
    char *last = malloc(128*sizeof(char));
    createGame(dirName, first, last);
    // free dynamically allocated memory
    playgame(dirName, first, last);
    free(first);
    free(last);
    free(dirName);
    return 0;
}
