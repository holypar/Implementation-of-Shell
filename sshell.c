#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dirent.h>

#define PATH_MAX 4096
// Found within https://stackoverflow.com/questions/9449241/where-is-path-max-defined-in-linux

#define CMDLINE_MAX 512


void executeCd(char* pathToChange){
        
        // Check if the path exists first 
        DIR *directoryPointer; 
        struct dirent *dp; 
        int canChange = 0; 
        // Doesn't work for absolute path right now. Need to change in the future. 
        directoryPointer = opendir("."); 

        while ((dp = readdir(directoryPointer)) != NULL) {
                if (!(strcmp(dp->d_name, pathToChange))) {
                        canChange = 1;
                        break; 
                }
        
        }
        if (canChange) {
                chdir(pathToChange);
                fprintf(stdout, "%s\n", "Changed Directory"); 
        } else {
                fprintf(stderr, "%s\n", "Error: cannot cd into directory");
        }

}

void executePwd() {
        char current_directory[PATH_MAX]; 
        getcwd(current_directory, sizeof(current_directory)); 
        fprintf(stdout, "%s\n", current_directory); 
        fprintf(stdout, "Return status value for '%s': %d\n", "pwd", 0);
}

// Function for executing a comand and parsing the command line 
// Source: Lecture from fork_exec_wait.c
// Input: User inputted command
// Output: exit status of execution

void splitCommandLine(char* command, char** args) {
        
        int tokenInteger = 0; 
        
        char* token = strtok(command, " "); 
        
        while (token != NULL) {

                args[tokenInteger] = token; 
                tokenInteger++;
                token = strtok(NULL, " ");
        }
        args[tokenInteger] = NULL;

        // Comments for phase 4 
        /* 
        Make sure to check that the number of arguements is less than 16 (command + number of args)
        ls folder > file.txt 
        Input: "ls", "folder", ">", "file.txt", NULL 
        P1:"ls", "folder", NULL 
        ">" "file.txt"
                1st Step: 
                Before: (STDOUT P1) -> terminal
                After: (STDOUT P1) -> file.txt 
                2nd Step: 
                Execute P1 
        ls folder > file.txt | cat file.txt
        "ls", "folder", "|", "cat", file.txt", NULL
        P1: "ls", "folder", NULL 
        P2: "cat", file.txt", NULL
        (STDOUT P1) -> (STDIN P2), (STDOUT) -> Terminal   
        */
         
}

/* 
void outputRedirection(char** args){

        char* leftHalf[17]; 
        char* rightHalf[17];
        // Finding the length of an array 
        int length = (sizeof args / sizeof args[0]);
        for (int i = 0; i < length; i++)
        {
                if (args[i] == ">") {
                        for (int j = 0; j < i; j++){
                                leftHalf[j] = args[j];
                        }
                        for (int j = (i+1); j < length; j++){
                                rightHalf[j] = args[j];
                                
                        }
                        
                } 
        }
        // P1 = lefthalf of > :  "ls", "folder", NULL
        // P2 = righthalf of > :  
}
*/ 


int ExecuteCommand(char** args) {
        
        pid_t pid;
        
        pid = fork();
        if (pid == 0) {
                // Child 
                execvp(args[0], args); 
                perror("execv");
                exit(1);
        } else if (pid > 0) {
                // Parent 
                int status; 
                waitpid(pid, &status, 0); 
                return WEXITSTATUS(status);
        } else {
                // Handle Errors 
                perror("fork");
                exit(1);
        }
}

int main(void)
{
        char cmd[CMDLINE_MAX];

        while (1) {
                char *nl;
                int retval;
                
                /* Print prompt */
                printf("sshell$ ");
                fflush(stdout);

                /* Get command line */
                fgets(cmd, CMDLINE_MAX, stdin);

                /* Print command line if stdin is not provided by terminal */
                if (!isatty(STDIN_FILENO)) {
                        printf("%s", cmd);
                        fflush(stdout);
                }

                /* Remove trailing newline from command line */
                nl = strchr(cmd, '\n');
                if (nl)
                        *nl = '\0';

                /* Builtin command */
                if (!strcmp(cmd, "exit")) {
                        fprintf(stderr, "Bye...\n");
                        break;
                }

                /* Builtin command for pwd */
                if (!strcmp(cmd, "pwd")) {
                        executePwd(); 
                        continue; 
                }

                char* args[17]; 
                splitCommandLine(cmd, args); 

                if (!strcmp(args[0], "cd")) {
                        executeCd(args[1]); 
                        continue; 
                }

                /* Regular command */
                retval = ExecuteCommand(args);
                fprintf(stdout, "Return status value for '%s': %d\n",
                         cmd, retval);
        }

        return EXIT_SUCCESS;
}


