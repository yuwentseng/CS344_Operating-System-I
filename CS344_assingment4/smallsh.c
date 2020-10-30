//Name: Yu-Wen, Tseng
//Courese: CS344
//Assignment4

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_ARGS 512    // maximum 512 arguments
#define MAX_CHARS 1028 //maximum 1028 characters per line

//the struct CmdFlags holds various status flags for the input and output filenames, for redirecting input or output, the count of arguments, and a flag for if a process is to be run in the background
struct CmdFlags
{
    bool background;// check if command is to be run in background
    char inputFile[MAX_CHARS];// filename of input file
    char outputFile[MAX_CHARS];// filename of output file
    char direct[MAX_CHARS];
    int argnumber;// keep count of number of arguments provided
    char *Arguments[MAX_ARGS];// maximum 512 arguments
};

//Modified stack for managing background processes
struct pidflags
{
    int backpidnum;
    pid_t backgp[MAX_ARGS];
}pids;

pid_t popoutpid();
pid_t topoutpid();

void launchpidflag();
void TurnPid(pid_t processId);
void DeletePid(pid_t processId);
int Shiftdirectoryfromhome(char* InputBuffer);
bool Headcharunique(char *str);
void Defineinput(char* InputBuffer,struct CmdFlags* Target);
void Launcharglist(struct CmdFlags* Target,char** Args);
void redirectset(struct CmdFlags* Target);
void workcommand(struct CmdFlags* Target);
void haltsignal(int sig);
void childsignal(int sig);
void termsignal(int sig);
void freeallspace(struct CmdFlags* Target);
void eliminateBGP();
void checkBG();
void implementshell();


//Globals
int finalforegroundstatus;
bool uniqueforeground = false;
//End Globals

int main(void)
{
    launchpidflag();
    implementshell(); // singleton for shell.
    return 0;
}

//Creates pid stack with -1 in each value.
void launchpidflag()
{
    int i;
    pids.backpidnum = -1;
    while(i < MAX_ARGS){
	 	pids.backgp[i] = -1;
		 i++;
	 }
}

//Adds a pid to the stack
void TurnPid(pid_t processId)
{
    pids.backgp[++(pids.backpidnum)] = processId;
}

//Delete a pid if an earlier background process ends.
void DeletePid(pid_t processId)
{
    int i , j;
    int pidPos;
    i= 0;
    while(i < pids.backpidnum + 1){
    	if(pids.backgp[i] == processId){
            pidPos = i;
            break;
        }
		i++;
	}
    j = pidPos;
    while(i <pids.backpidnum + 1){
    	pids.backgp[i] = pids.backgp[i+1];
		i++;
	}
    pids.backpidnum--;
}

//Pop out pid from top
pid_t popoutpid()
{
    return pids.backgp[pids.backpidnum--];
}

//Reads top of stack
pid_t topoutpid()
{
    return pids.backgp[pids.backpidnum];
}

//Allow user to find file directory
int Shiftdirectoryfromhome(char* InputBuffer)
{
    char* HomeDirectoryPath = getenv("HOME"); //Gets home path.
    char NewPath[MAX_CHARS]; //NewPath[1028]

    InputBuffer[strlen(InputBuffer) -1] = '\0';

    if(strcmp(InputBuffer,"cd") == 0){
        if(chdir(HomeDirectoryPath) != 0){ // Cannot find directory
            printf("Directory:%s not found.\n",HomeDirectoryPath);
            return 1;
        }
        return 0;
    }

    memset(NewPath,'\0',sizeof(NewPath));
    strtok(InputBuffer," "); // Removing useless spacing
    strcpy(InputBuffer,strtok(NULL,""));
    
    //printf("    %s\n",InputBuffer);
    int i ;
  	i = (InputBuffer[0] == '/')?0:-1;
  	if(!(InputBuffer[0] == '/')){
  		i = (strcmp(InputBuffer,"..") == 0)?1:-1;
  		if(!(strcmp(InputBuffer,"..") == 0)){
  			i = (strcmp(InputBuffer,"~") == 0)?2:-1;
  			if(!(strcmp(InputBuffer,"~") == 0)){
  				i = (InputBuffer[0] == '.' && InputBuffer[1] == '/')?3:-1;
  				if(!(InputBuffer[0] == '.' && InputBuffer[1] == '/')){
  					i = 4;	
				  }
			  }
		  }
	}
	switch(i){
        case 0:
            sprintf(NewPath,"%s%s",HomeDirectoryPath,InputBuffer); //Forward to a specifed directory from home directory//printf("Path: %s\n",NewPath)
            break;
        case 1:
            strcpy(NewPath,InputBuffer);//Turn back to a folder//printf("Path: %s\n",NewPath)
            break;
        case 2:
             strcpy(NewPath,HomeDirectoryPath);//Turn back to a folder//printf("Path: %s\n",NewPath)
             break;
        case 3:
            sprintf(NewPath,"%s",InputBuffer);//Indicate current directory//printf("Path: %s\n",NewPath)
            break;
        case 4:
      		 sprintf(NewPath,"%s",InputBuffer); // goto directory from home//printf("Path: %s\n",NewPath);
      		 break;
    }
    if(chdir(NewPath) != 0){ // cannot find directory
        printf("Directory:%s not found.\n",NewPath);
        return 1;
    }
    return 0;
}

//Function to check for '&','<','>','#' and prevent additonal arguments
bool Headcharunique(char *str)
{
    bool BeUnique = false;
    if(str == NULL){ //Make sure value isn't null so the rest of status don't segment fault
        return true;
    }
	switch (str[0]){ //Checking every case if it happens
		case '&': // Check for background
			BeUnique = true;
			break;
		case '<': // Check for input character
			BeUnique = true;
			break;
		case '>': // Check for output character
			BeUnique = true;
			break;
		case '#': // Check for command character
            BeUnique = true;
			break;
	}
    return BeUnique;
}

//Initialize the struct that holds all arguments.
void Defineinput(char* InputBuffer,struct CmdFlags* Target)
{
    char Buffer[MAX_CHARS];
    char *InputFileName;
    char *OutputFileName;
    char *Temp;

    Target->argnumber = 0;
    InputBuffer[strlen(InputBuffer) -1] = '\0'; // Delete '\n'
	
	switch (InputBuffer[strlen(InputBuffer) -1]){ //Checking for two case
		case '&': // Check for background enabled
            Target->background = true;
            InputBuffer[strlen(InputBuffer) -1] = '\0'; // Delete character //printf("background: enabled\n");
        break;
        default :
        	Target->background = false; //printf("background: disabled\n");
	}
    memset(Buffer,'\0',sizeof(Buffer)); // clear Buffer
    strncpy(Buffer,InputBuffer,MAX_CHARS); //Copy buffer
    strtok(Buffer," "); // Only take the part of the input command
    strcpy(Target->direct,Buffer); // take command place in new obj.
    //printf("direct: %s\n",Target->direct);

    //InputFile Name
    memset(Buffer,'\0',sizeof(Buffer)); 
    strncpy(Buffer,InputBuffer,MAX_CHARS);
    InputFileName = strstr(Buffer,"<"); // Take everything after "<"
    int ck;
    ck = (InputFileName != NULL)?1:-1;
    switch (ck){
    	case 1:
    		memmove(InputFileName, InputFileName+2, strlen(InputFileName)); //Put everything but "< " in string
            strtok(InputFileName," "); // Remove excess
        InputFileName[strlen(InputFileName)] = '\0'; // Add end character
        strcpy(Target->inputFile,InputFileName);
		break;	
	}
    //OutputFile Name;
    memset(Buffer,'\0',sizeof(Buffer));
    strncpy(Buffer,InputBuffer,MAX_CHARS);
    OutputFileName = strstr(Buffer,">"); // Take everything after ">"
    int ck2;
    ck2 = (OutputFileName != NULL)?1:-1;
	switch (ck2){
		case 1:
		memmove(OutputFileName, OutputFileName+2, strlen(OutputFileName));//Put everything but "> " in string
        strtok(OutputFileName," "); // Remove excess
        OutputFileName[strlen(OutputFileName)] = '\0'; // Add end character
        strcpy(Target->outputFile,OutputFileName);
		break;	
	}
    //Arguments
    memset(Buffer,'\0',sizeof(Buffer));
    strncpy(Buffer,InputBuffer,MAX_CHARS);
    strtok(Buffer," "); // Take everything before first space
    Temp = strtok(NULL,""); // Take everyhting after first space
    //printf("Temp Line:%s\n",Temp)
    if(Headcharunique(Temp) == false){ // check if there are any arguments
        strcpy(Buffer,Temp);
        strtok(Buffer,"<>&#"); // Take everything before arguments
        
        strtok(Buffer," "); // Remove space
        Target->Arguments[0] = Buffer; // Direct to first argument
        Target->argnumber = 1;
        Temp = strtok(NULL," "); // Direct to next argument
        while(Temp != NULL){
            Target->Arguments[Target->argnumber] = Temp;// Put all the arguments into the list.
            Target->argnumber++;
            Temp = strtok(NULL," ");
        }
        Target->Arguments[Target->argnumber] = strtok(NULL, ""); //Take the last argument.
    }
}

//Creates list of arguments
void Launcharglist(struct CmdFlags* Target,char** Args)
{
    int i;
	i= 0;
    Args[0] = Target->direct; // First arg is command itself.
    while (i <Target->argnumber){
    	if(getenv(Target->Arguments[i]) != NULL){
            Args[i+1] = getenv(Target->Arguments[i]); // Put into all arguments.
        }
        else if(strcmp(Target->Arguments[i],"$$") == 0){
            sprintf(Args[i+1],"%d",getpid());
        }
        else{
            Args[i+1] = (Target->Arguments[i]);
        }
		i++;
	}
    Args[i+1] = NULL;
}

//Creates redirection of input and output
void redirectset(struct CmdFlags* Target)
{
    int InputFileDescriptor = STDIN_FILENO;
    int OutputFileDescriptor = STDOUT_FILENO;
	int temp;
    if(Target->inputFile[0] != '\0'){ // Check if inputs are active.
        //printf("INPUT: %s\n",Target->inputFile);
        InputFileDescriptor = open(Target->inputFile,O_RDONLY); // Open file.
		temp = 1;
		if(temp ==1){
			if(InputFileDescriptor < 0){ // If not found exit.
            printf("File cannot be found.\n");
            exit(1);
        	}	
		}
        dup2(InputFileDescriptor,0); // Shift input redirection.
        close(InputFileDescriptor); // Close file.
    }
    if(Target->outputFile[0] != '\0'){ // Check if outputs actve.
        //printf("OUTPUT: %s\n",Target->outputFile);
        OutputFileDescriptor = open(Target->outputFile,O_WRONLY | O_CREAT | O_TRUNC,0644); // Create new file or edit.
		temp = 2;
		if(temp ==2){
			 if(OutputFileDescriptor < 0){ // Check for error.
            printf("Error opening or creating file.");
            exit(1);
        	}
		}
        dup2(OutputFileDescriptor,1);//Shift output directions.
        close(OutputFileDescriptor);
    }
}

//Creates a child fork for a command
void workcommand(struct CmdFlags* Target)
{
    pid_t pid = fork();
    char *ArgList[MAX_ARGS];
    int ProcessStatus;
	if(pid == -1){ //Error
		printf("Something went wrong with fork().\n");
            exit(1);
	}
	else if (pid ==0){
		 redirectset(Target);

            Launcharglist(Target,ArgList);
            execvp(Target->direct, ArgList); // Implement command.
            
            printf("direct not found.\n");
            exit(1);
	}
	else{
		if(Target->background == true && uniqueforeground == false){ //Setup background or non background.
                TurnPid(pid); // Put into background stack.
                printf("background Pid is %d\n",topoutpid());
            }
            else{
                
                waitpid(pid,&ProcessStatus,0); // Wait the shell when background inactive.
                finalforegroundstatus = ProcessStatus; //printf("parent(%d) waited for child process(%d)\n",getpid() ,pid);
            }	
	}
}

//Learn following function from: https://piazza.com/class/ixhzh3rn2la6vk?cid=365
void haltsignal(int sig)
{	
	int ck = (uniqueforeground==false)?1:-1;
	char * message;
	switch (ck ){
		case 1:
			message = ("\nEntering foreground-only mode (& is now ignored)\n"); // Enable Foregroundmode.
        write(STDOUT_FILENO, message, 50);
        uniqueforeground = true; // Change global.
        break;
        default:
        	message = "\nExiting foreground-only mode\n"; // Exit Foreground mode.
        write(STDOUT_FILENO, message, 31);
        uniqueforeground = false; // Change gloabl.
	}
    
}

//Learn following function from: http://stackoverflow.com/questions/2377811/tracking-the-death-of-a-child-process
void childsignal(int sig)
{
    //SOURCE: http://carpediem101.com/wp-content/uploads/it-works-why.jpg
    pid_t ChildPid;
    int ChildStatus;
    int i;
	i = 0;
	while (i <pids.backpidnum + 1 ){
		ChildPid = waitpid(pids.backgp[i],&ChildStatus,WNOHANG);

        if(ChildPid != 0 ){ // if exited or errored on exit.
        	if(ChildStatus == 0){
            	fprintf(stdout,"\nbackground pid %d is done: exit value %d\n",ChildPid,ChildStatus);
            	DeletePid(ChildPid);
        	}
        	else if (ChildStatus ==1){ // If exited or errored on exit.
        		fprintf(stdout,"\nbackground pid %d is done: exit value %d\n",ChildPid,ChildStatus);
            	DeletePid(ChildPid);
			}
        }
        else if(ChildPid != 0){ // Other signals.
            fprintf(stdout,"\nbackground pid %d is done: terminated by signal %d\n", ChildPid, ChildStatus);
            DeletePid(ChildPid);
        }
		i++;
	}
}

//Signal handler for ^c of a foreground process.
void termsignal(int sig)
{
    printf("\nterminated by signal %d\n",sig); 
}

//Function to free input object.
void freeallspace(struct CmdFlags* Target)
{
    Target->background = false; // reset background
    //clear all fields.
    memset(Target->inputFile,'\0',sizeof(Target->inputFile));
    memset(Target->outputFile,'\0',sizeof(Target->outputFile));
    memset(Target->direct,'\0',sizeof(Target->direct));
    free(Target);
}

//Function for exting.
void eliminateBGP()
{
    int i;
    i = 0;
    while (i <pids.backpidnum + 1){
    	kill(pids.backgp[i], SIGINT); // Interrupt all background pids.
    	i++;
	}
}

//Alter foreground modes if a program returns a stop signals.
void checkBG()
{
    if(WTERMSIG(finalforegroundstatus) == 11 && uniqueforeground == true){ // If signal is stop and foreground true switch foreground mode.
        printf("\nExiting foreground-only mode\n");
        uniqueforeground = false;
    }
    else if(WTERMSIG(finalforegroundstatus) == 11 && uniqueforeground == false){ // If signal is stop and foreground false switch foreground mode.
        printf("\nEntering foreground-only mode (& is now ignored)\n");
        uniqueforeground = true;
    }
}

//Implement the shell itself.
void implementshell()
{
    char InputBuffer[MAX_CHARS];
    struct CmdFlags *Target;
    int ForegroundStatus;
    //Initialize the signals
    struct sigaction StopSignal;
    StopSignal.sa_handler = haltsignal;
    StopSignal.sa_flags = 0;

    struct sigaction TermSignal;
    TermSignal.sa_handler = termsignal;
    StopSignal.sa_flags = 0;

    struct sigaction ChildSignal;
    ChildSignal.sa_handler = childsignal;
    StopSignal.sa_flags = 0;
    //End initial signals

    do
    {
        //Reseting signal handlers.
        sigaction(SIGTSTP,&StopSignal, NULL);
        sigaction(SIGINT,&TermSignal, NULL);
        sigaction(SIGCHLD,&ChildSignal, NULL);
        // printf("STATUS: %d FGMODE:%d\n",WTERMSIG(finalforegroundstatus),uniqueforeground);
        checkBG();
        //Clearing stdin and out.
        fflush(stdout);
        fflush(stdin);
        printf(": ");
        memset(InputBuffer,'\0',sizeof(InputBuffer));
        fgets(InputBuffer,sizeof(InputBuffer),stdin); // Get command line.
        fflush(stdout);
        fflush(stdin);
		
		int i;
		i = (strncmp(InputBuffer,"exit",4) == 0)?0:-1;
		if (!strncmp(InputBuffer,"exit",4) == 0){ // Exit shell. //printf("Exiting....\n");
			i = (strncmp(InputBuffer, "#",1) == 0)?1:-1;
			if(!(strncmp(InputBuffer, "#",1) == 0)){
				i = (strncmp(InputBuffer,"cd", 2) == 0)?2:-1;
				if (!(strncmp(InputBuffer,"cd", 2) == 0)){ // Change directory //printf("Changing directory...\n");
					i = (strncmp(InputBuffer,"status",6) == 0)?3:-1;
					if(!(strncmp(InputBuffer,"status",6) == 0)){ // Check final foreground command status.
						i = 4;
					}
				}
			} 
		}
		switch (i){
			case 0:
				eliminateBGP();
            exit(0);
            break;
            case 1:
            	continue;
            	break;
            case 2:
            	Shiftdirectoryfromhome(InputBuffer);
            	break;
            case 3:
            	if(WEXITSTATUS(finalforegroundstatus)){
                ForegroundStatus = WEXITSTATUS(finalforegroundstatus); // Check if extied.
            }
            else{
                ForegroundStatus = WTERMSIG(finalforegroundstatus); // check if terminated by signal.
            }
            printf("exit value %d\n",ForegroundStatus);
            break;
            case 4:
            	if(InputBuffer != NULL && strcmp(InputBuffer,"") != 0){
                //read in a command.
                Target = malloc(1 * sizeof(struct CmdFlags));
                Defineinput(InputBuffer,Target); // Define command line.
                workcommand(Target); // Implement command.

                freeallspace(Target);
            }
            else{
                continue;
            }
            break;
		}
    }
    while(true);
}


