#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dirent.h>
#include <stdbool.h>
#define PATH_MAX 4096
// Found within https://stackoverflow.com/questions/9449241/where-is-path-max-defined-in-linux
#define CMDLINE_MAX 512
#define MAX_TOKENS 17 
#define MAX_PROCESS 4
struct Process {
        char* args[MAX_TOKENS];
        bool redirection;
        char* fileName; 
}; 


void ExecuteCd(char* pathToChange){
        
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

void ExecutePwd() {
        char current_directory[PATH_MAX]; 
        getcwd(current_directory, sizeof(current_directory)); 
        fprintf(stdout, "%s\n", current_directory); 
        fprintf(stdout, "Return status value for '%s': %d\n", "pwd", 0);
}

// Function for executing a comand and parsing the command line 
// Source: Lecture from fork_exec_wait.c
// Input: User inputted command
// Output: exit status of execution

int SplitCommandLine(char* command, char** args) {
        
        int tokenInteger = 0; 
        
        char* token = strtok(command, " "); 
        
        while (token != NULL) {

                args[tokenInteger] = token; 
                tokenInteger++;
                token = strtok(NULL, " ");
        }
        args[tokenInteger] = NULL;
        return tokenInteger; 
}

struct Process createProcess(char** processTokens, int tokensLength) {
        /* Create a process struct*/
        struct Process process; 
        
        /* Create local variables and properties*/
        int argsToken = 0; 
        bool redirection = false;
        char* filename = ""; 
        char* args[MAX_TOKENS];

        /* Go through the tokens */
        for (int i = 0; i < tokensLength; i++)
        {
                /* Reach a > and update properties */
                if (!strcmp(processTokens[i], ">")) {
                        redirection = true;
                        filename = processTokens[i + 1];
                        break;  
                } 
                /* Otherwise store the token */
                args[i] = processTokens[i];
                argsToken++;  
        }

        /* Store the tokens into the struct */
        for (int i = 0; i < argsToken; i++)
        {
                process.args[i] = args[i]; 
        }
        /* Store the properties into the struct */
        process.args[argsToken + 1] = NULL; 
        process.fileName = filename; 
        process.redirection = redirection; 
        
        return process; 

}

void ParseCommandLine(char* command, struct Process* processList) {
        /*Splitting the command line into tokens */
        char* splitTokens[MAX_TOKENS]; 
        int tokensLength = SplitCommandLine(command, splitTokens); 

        /* Going through each token and create a process */
        int numberProcess = 0; 
        int startCounter = 0; 
        
        for (int i = 0; i < tokensLength; i++)
        {
                if (!strcmp(splitTokens[i], "|")) {
                        char* processTokens[MAX_TOKENS];
                        int numberTokens = 0; 
                        /* Copy everything up to the pipe */
                        for (int j = startCounter; j < i-1; j++)
                        {
                                processTokens[j] = splitTokens[j]; 
                                numberTokens++; 
                        }

                        /* Create a process struct and store it */
                        struct Process process = createProcess(processTokens, numberTokens);
                        processList[numberProcess] = process; 
                        numberProcess++; 

                        /* Reset the starting counter for the process */
                        startCounter = (i++); 
                }
        }

        /* Storing the last process */
        char* processTokens[MAX_TOKENS];
        int numberTokens = 0;
        for (int j = startCounter; j < tokensLength; j++)
        {
                processTokens[j] = splitTokens[j];
                numberTokens++;  
        }
        
        struct Process process = createProcess(processTokens, numberTokens);
        processList[numberProcess] = process; 
        numberProcess++; 
} 

int ExecuteCommand(struct Process process) {
        
        pid_t pid;
        
        pid = fork();
        if (pid == 0) {
                // Child 
                execvp(process.args[0], process.args); 
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
                        ExecutePwd(); 
                        continue; 
                }
                
                
                struct Process processList[MAX_PROCESS]; 
                ParseCommandLine(cmd, processList);
                struct Process first_process = processList[0]; 

                if (!strcmp(first_process.args[0], "cd")) {
                        ExecuteCd(first_process.args[1]); 
                        continue; 
                }

                /* Regular command */
                retval = ExecuteCommand(first_process);
                fprintf(stdout, "Return status value for '%s': %d\n",
                         cmd, retval);
        }

        return EXIT_SUCCESS;
}


