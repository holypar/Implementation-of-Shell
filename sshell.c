#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dirent.h>
#include <stdbool.h>
#include <fcntl.h>

#define PATH_MAX 4096
// Found within https://stackoverflow.com/questions/9449241/where-is-path-max-defined-in-linux
#define CMDLINE_MAX 512
#define MAX_TOKENS 17 
#define MAX_PROCESS 4
#define MAX_TOKEN_LENGTH 32


//fd = open(filename.txt, O_CREAT | O_TRUNC | O_WRONLY)
// fd = open("myfile.txt", O_WRONLY | O_CREAT, 0644)
//dup2(fd, STDOUT_FILENO)



/* Data Structure for Processes*/
struct Process {
        char* args[MAX_TOKENS];
        bool redirection;
        char* fileName; 
}; 

/* Built in command for cd */
int ExecuteCd(char* pathToChange){
        /* Checking to see if chdir fails */
        if (chdir(pathToChange) != 0) 
                return 1; 
        return 0; 
}

/* Built in command for pwd */
void ExecutePwd() {
        /* Creating maximum string to store file path */
        char current_directory[PATH_MAX]; 
        getcwd(current_directory, sizeof(current_directory)); 

        /* Printing the output the terminal */
        fprintf(stdout, "%s\n", current_directory); 
        fprintf(stdout, "+ completed 'pwd' [0]\n");
}

int SplitCommandLine(char* command, char** args) {
        
        unsigned int tokenInteger = 0;  
        char* token = strtok(command, " "); 
        bool needsToStore = true; 

        /* Going through each token one by one */
        while (token != NULL) {
                for (unsigned int i = 0; i < strlen(token); i++ ){
                        /* Specific Edge Case if symbol is inside token */
                        // echo Hello World > file.txt 
                        if ((token[i] == '>' || token[i] == '|') && (strlen(token) > 1)) {
                                /* Splitting the string before and after the symbol*/
                                // >, |, >&, |&,
                                char beforeSymbol[MAX_TOKEN_LENGTH] = "";
                                char afterSymbol[MAX_TOKEN_LENGTH] = "";
                                char symbol[2]; 

                                /* Copies everything before the symbol and after the symbol*/
                                for (unsigned int j = 0; j < i; j++){
                                        beforeSymbol[j] = token[j];
                                }
                                for (unsigned int j = i+1; j < strlen(token); j++){
                                        afterSymbol[j-(i+1)] = token[j];
                                }   
                                
                                /* Storing the 3 separate tokens */
                                args[tokenInteger] = beforeSymbol;
                                tokenInteger++;

                                symbol[0] = token[i]; 
                                symbol[1] = '\0'; 
                                args[tokenInteger] = symbol;
                                tokenInteger++;

                                args[tokenInteger]= afterSymbol;
                                tokenInteger++;
                                
                                /* Token no longer needs to be stored */
                                needsToStore = false; 
                                
                        }
                }
                /* If token needs to be stored, store it */
                if (needsToStore) {
                        args[tokenInteger] = token; 
                        tokenInteger++;
                        
                }
                /* Gets the next token and updates the needs to be stored boolean */
                token = strtok(NULL, " ");
                needsToStore = true; 
                
        }
        /* Storing the last token as NULL */
        return tokenInteger; 
}

struct Process createProcess(char** processTokens, int tokensLength) {
        /* Create a process struct */
        struct Process process; 
        
        /* Create local variables and properties */
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
        process.args[argsToken] = NULL; 
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
                /* If the process has a redirection, redirect it to process with the filename */
                if (process.redirection) {
                        int fd; 
                        
                        /* Opens the filename and redirects the stream to the file */
                        fd = open(process.fileName, O_WRONLY | O_CREAT, 0644);
                        dup2(fd, STDOUT_FILENO); 
                        close(fd);  
                }
                execvp(process.args[0], process.args); 
                perror("execvp");
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
                        fprintf(stdout, "+ completed 'exit' [0]\n");
                        break;
                }

                /* Builtin command for pwd */
                if (!strcmp(cmd, "pwd")) {
                        ExecutePwd(); 
                        continue; 
                }
                
                struct Process processList[MAX_PROCESS]; 

                /* Copies the commandline for later use */
                char copycmd[CMDLINE_MAX];
                strcpy(copycmd, cmd); 

                /* Parses the command line into processes */
                ParseCommandLine(cmd, processList);
                struct Process first_process = processList[0]; 

                /* Builtin command for cd */
                if (!strcmp(first_process.args[0], "cd")) {
                        retval = ExecuteCd(first_process.args[1]);
                        fprintf(stdout, "+ completed '%s' [%d]\n",
                        copycmd, retval);
                        continue;  
                }

                /* Regular command */
                retval = ExecuteCommand(first_process);
                fprintf(stdout, "+ completed '%s' [%d]\n",
                        copycmd, retval);
        }

        return EXIT_SUCCESS;
}


