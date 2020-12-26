#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>

int main(int argc, char**argv){
	char prompt[20] = "308sh> "; // variable to assign my shell's command line
	char nextCmd[1024];  // the command user enters in my shell every iteration
	char* cmdBack; // a variable to keep track of command name running in the background
	int status; // a variable to record the status of a process
	if(argc != 1 && argc !=3){ // case when user does not enter valid information in the shell
		printf("Error\n");
		exit(0);
	}else if(argc ==3){ //  to give the user an option to change the shell name
		if(strcmp(argv[1],"-p")!=0){
			printf("Error\n");
			return -1;
		}else{
			strcpy(prompt, strcat(argv[2], "> "));
		}
	}

	while(1){ // a constant loop to get user's commands
		
		//get users's command
		printf("%s", prompt); 
		fgets(nextCmd, 1024, stdin);
		nextCmd[strlen(nextCmd)-1] = '\0';
		
		// check if the current iteration has a background processing ending
		int tempPID = waitpid(-1, &status, WNOHANG);
		if( tempPID > 0){			
			printf("[%d] %s Exit %d\n", tempPID, cmdBack, WEXITSTATUS(status));
			//isBackground =0;
		}
		
		// cases for running built-in or programmable commands based on user input
		if(strcmp(nextCmd,"exit") ==0 ){ // case to exit out of the shell
			exit(0);
		}
		else if(strcmp(nextCmd,"pid") ==0 ){ // case to output PID
			printf("%d\n",getpid());
		}
		else if(strcmp(nextCmd,"ppid") ==0 ){ // case to output PPID
			printf("%d\n",getppid());
		}
		else if(strcmp(nextCmd,"pwd") ==0 ){ // case to output current working directory
			char dir[1024];
			getcwd(dir, sizeof(dir));
			printf("%s\n",dir);
		}
		else if(strncmp(nextCmd,"cd",2) ==0 ){ // case to change directory
			strtok(nextCmd, " ");
			char *directory;
			directory = strtok(NULL, " ");
			if(directory != NULL){ // check if the user has a specific directory to change to 
				chdir(directory);
			}else{
				chdir("/home");
			}	
		}else{ // case for programmable commands
			int isBackground =0; // variable to check if a process is a background process
			if(strchr(nextCmd, '&')){ // check if the current is a background process
				isBackground =1;
				int l;
				for( l =0;l<1024;l++){
					if( nextCmd[l] == '&'){
						nextCmd[l] = '\0';
					}
				}		
			}

			char* validParts[10]; // variable to track all the parts of a command
			validParts[0] = strtok(nextCmd, " "); // record command name
			if( isBackground == 1){ // record the command name if it is background process
				cmdBack = validParts[0];
			}
			char* part = strtok(NULL, " ");
			int i=1;
			int k;
			for(k=1;k<10;k++){ // initialize rest of the character array
				validParts[k] = (char*) NULL;
			} 
			while(part != NULL){ // record all the parts of the command entered
				validParts[i] =part;
				part = strtok(NULL, " ");
				i++;			
			}

			int pid,pid2; // variable to keep track of the pid of the child process.			
			pid = fork(); // start a child process 
		if( isBackground ==  0){
			if ( pid ==0 ){ // is child process 
				int processPID = getpid();
				printf("[%d] %s\n",processPID,validParts[0]);
				execvp(validParts[0],validParts);
				perror('\0');
				printf("Command failed or was not found\n");
                                printf("[%d] %s Exit 255\n", processPID, validParts[0]);
			}else{ // is parent process
				usleep(1000);
				int outPID =waitpid(pid, &status, 0);
				printf("[%d] %s Exit %d\n", outPID, validParts[0], WEXITSTATUS(status));	
			}
		}else{
		
			if ( pid ==0 ){ // is child process 
                                pid2 =fork();
				if( pid2 == 0){
					int processPID = getpid();
                                	printf("[%d] %s\n",processPID,validParts[0]);
                                	execvp(validParts[0],validParts);
                                	perror('\0');
                               	 	printf("Command failed or was not found\n");
                                	printf("[%d] %s Exit 255\n", processPID, validParts[0]);
				}else{
				
					usleep(1000);
					int backPID = waitpid(pid2, &status, 0);
                                        printf("[%d] BackProcess Exit %d\n", backPID, WEXITSTATUS(status));
                                        isBackground =0;
				}
                        }else{ // is parent process
                                usleep(200);
                        }

		}		
		}
	}
}
