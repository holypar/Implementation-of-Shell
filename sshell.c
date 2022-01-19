#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdbool.h>
#include <fcntl.h>

#define PATH_MAX 4096
// Found within https://stackoverflow.com/questions/9449241/where-is-path-max-defined-in-linux
#define CMDLINE_MAX 512
#define MAX_TOKENS 17
#define MAX_PROCESS 4
#define MAX_TOKEN_LENGTH 32
#define ERROR_NUMBER 1
#define SUCCESS_NUMBER 0

/* Data Structure for Processes */
struct Process
{
        char *args[MAX_TOKENS];
        bool redirection;
        bool errorRedirect;
        char *fileName;
};

/* Data Structure for Process Logistics */
struct ProcessLogic
{
        int outputCode;
        int numberProcesses;
};

/* Built in command for cd */
int ExecuteCd(char *pathToChange)
{
        /* Checking to see if chdir fails */
        if (chdir(pathToChange) != 0)
        {
                fprintf(stderr, "Error: cannot cd into directory\n");
                return ERROR_NUMBER;
        }

        return SUCCESS_NUMBER;
}

/* Built in command for pwd */
void ExecutePwd()
{
        /* Creating maximum string to store file path */
        char currentDirectory[PATH_MAX];
        getcwd(currentDirectory, sizeof(currentDirectory));

        /* Printing the output the terminal */
        fprintf(stdout, "%s\n", currentDirectory);
}

/* Built in command for sls */
void ExecuteSls()
{
        /* Creating local variables for storage */
        long long fileSize;
        DIR *currentDirectory;
        struct stat statistic;
        struct dirent *dp;

        /* Opening the current directory and iterating through the directory */
        currentDirectory = opendir(".");
        while ((dp = readdir(currentDirectory)) != NULL)
        {
                /* Getting the current file statistic and printing it */
                if (dp->d_name[0] == '.')
                        continue;
                stat(dp->d_name, &statistic);
                fileSize = statistic.st_size;
                printf("%s (%lld bytes)\n", dp->d_name, fileSize);
        }
}
/*Function that splits the command line into tokens so they can be later on used to create processes */
int SplitCommandLine(char *command, char **args)
{

        unsigned int tokenInteger = 0;
        char *token = strtok(command, " ");
        bool needsToStore = true;

        /* Going through each token one by one */
        while (token != NULL)
        {
                for (unsigned int i = 0; i < strlen(token); i++)
                {
                        /* Specific Edge Case if symbol is inside token */
                        // echo Hello World > file.txt
                        if ((token[i] == '>' || token[i] == '|') && (strlen(token) > 2))
                        {
                                /* Splitting the string before and after the symbol */
                                int offset = 1;
                                if (token[i + 1] == '&')
                                {
                                        offset++;
                                }

                                char beforeSymbol[MAX_TOKEN_LENGTH] = "";
                                char afterSymbol[MAX_TOKEN_LENGTH] = "";
                                char symbol[3];

                                for (unsigned int j = 0; j < i; j++)
                                {
                                        beforeSymbol[j] = token[j];
                                }
                                for (unsigned int j = i + offset; j < strlen(token); j++)
                                {
                                        afterSymbol[j - (i + offset)] = token[j];
                                }

                                /* Storing the 3 separate tokens */
                                args[tokenInteger] = beforeSymbol;
                                tokenInteger++;

                                /* Saves the >& into a string */
                                if (token[i + 1] == '&')
                                {
                                        symbol[0] = token[i];
                                        symbol[1] = token[i + 1];
                                        symbol[2] = '\0';
                                }
                                else
                                {
                                        symbol[0] = token[i];
                                        symbol[1] = '\0';
                                }

                                /* Stores either the > or the >& */
                                args[tokenInteger] = symbol;
                                tokenInteger++;

                                args[tokenInteger] = afterSymbol;
                                tokenInteger++;

                                /* Token no longer needs to be stored */
                                needsToStore = false;
                        }
                }
                /* If token needs to be stored, store it */
                if (needsToStore)
                {
                        args[tokenInteger] = token;
                        tokenInteger++;
                }
                /* Gets the next token and updates the needs to be stored boolean */
                token = strtok(NULL, " ");
                needsToStore = true;
        }
        args[tokenInteger] = NULL;
        tokenInteger++;
        /* Storing the last token as NULL */
        return tokenInteger;
}

struct Process createProcess(char **processTokens, int tokensLength)
{
        /* Create a process struct */
        struct Process process;

        /* Create local variables and properties */
        int argsToken = 0;
        bool redirection = false;
        bool errorRedirect = false;
        char *filename = "";
        char *args[MAX_TOKENS];

        /* Go through the tokens */
        for (int i = 0; i < tokensLength; i++)
        {
                /* Reach a > and update properties */
                if (!strcmp(processTokens[i], ">") || !strcmp(processTokens[i], ">&"))
                {
                        if (!strcmp(processTokens[i], ">&"))
                                errorRedirect = true;
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
        process.errorRedirect = errorRedirect;
        return process;
}

/* Helper functions that are used for the CheckParsing function */

/* Checks if there is no command after a '|' or if there is an instance of having back to back pipes '||' */
int CheckMissingCommand(char *token, char *nextToken)
{
        return (((!strcmp(token, "|")) && (nextToken == NULL)) || ((!strcmp(token, "|")) && (!strcmp(nextToken, "|"))));
}
/* Checks if there is no output file */
int CheckNoOutputFile(char *token, char *nextToken)
{
        return (((!strcmp(token, ">")) && (nextToken == NULL)) || ((!strcmp(token, ">")) && (!strcmp(nextToken, "|"))));
}
/* Checks if there is an output file to work with */
int CheckOutputFile(char *token, char *nextToken)
{
        return (!strcmp(token, ">") && nextToken != NULL && (strcmp(nextToken, "|")));
}

int CheckParsing(char **splitTokens, int tokensLength)
{
        if (tokensLength > 16)
        {
                fprintf(stderr, "Error: too many process arguments\n");
                return ERROR_NUMBER;
        }
        /* If first command is a '|' or '>' */
        if (!strcmp(splitTokens[0], "|") || !strcmp(splitTokens[0], ">") || !strcmp(splitTokens[0], " "))
        {
                fprintf(stderr, "Error: missing command\n");
                return ERROR_NUMBER;
        }
        bool redirectionFound = false;
        for (int i = 0; i < tokensLength - 1; i++)
        {
                if (redirectionFound && !strcmp(splitTokens[i], "|"))
                {
                        fprintf(stderr, "Error: mislocated output redirection\n");
                        return ERROR_NUMBER;
                }

                if ((((!strcmp(splitTokens[i], "|")) && (splitTokens[i+1] == NULL)) || ((!strcmp(splitTokens[i], "|")) && (!strcmp(splitTokens[i+1], "|")))))
                {
                        fprintf(stderr, "Error: missing command\n");
                        return ERROR_NUMBER;
                }

                if ((((!strcmp(splitTokens[i], ">")) && (splitTokens[i+1] == NULL)) || ((!strcmp(splitTokens[i], ">")) && (!strcmp(splitTokens[i+1], "|")))))
                {
                        fprintf(stderr, "Error: no output file\n");
                        return ERROR_NUMBER;
                }
                if ((!strcmp(splitTokens[i], ">") && splitTokens[i+1] != NULL && (strcmp(splitTokens[i+1], "|"))))
                {

                        /* Found function on https://stackoverflow.com/questions/230062/whats-the-best-way-to-check-if-a-file-exists-in-c*/
                        /* Checks if a file exists */
                        if (access(splitTokens[i + 1], F_OK) == 0)
                        {
                                int fd = open(splitTokens[i + 1], O_WRONLY, 0644);
                                if (fd == -1)
                                {
                                        fprintf(stderr, "Error: cannot open output file\n");
                                        close(fd);
                                        return ERROR_NUMBER;
                                }
                        }
                }
                if (!strcmp(splitTokens[i], ">"))
                        redirectionFound = true;
        }
        return SUCCESS_NUMBER;
}

struct ProcessLogic ParseCommandLine(char *command, struct Process *processList)
{
        /*Splitting the command line into tokens */
        char *splitTokens[MAX_TOKENS];
        int tokensLength = SplitCommandLine(command, splitTokens);
        struct ProcessLogic logicstics;
        // Check all parsing errors here:
        int returnCode = CheckParsing(splitTokens, tokensLength);

        if (returnCode)
        {
                logicstics.numberProcesses = 0;
                logicstics.outputCode = returnCode;
                return logicstics;
        }

        /* Going through each token and create a process */
        int numberProcess = 0;
        int startCounter = 0;

        for (int i = 0; i < tokensLength - 1; i++)
        {
                if (!strcmp(splitTokens[i], "|") || !strcmp(splitTokens[i], "|&"))
                {
                        char *processTokens[MAX_TOKENS] = {};
                        int numberTokens = 0;
                        /* Copy everything up to the pipe */
                        for (int j = startCounter; j < i; j++)
                        {
                                processTokens[j-startCounter] = splitTokens[j];
                                numberTokens++;
                        }

                        /* Create a process struct and store it */
                        struct Process process = createProcess(processTokens, numberTokens);
                        if (!strcmp(splitTokens[i], "|&"))
                        {
                                process.errorRedirect = true;
                        }
                        processList[numberProcess] = process;
                        numberProcess++;

                        /* Reset the starting counter for the process */
                        startCounter = (i + 1);
                }
        }

        /* Storing the last process */
        char *lastTokens[MAX_TOKENS] = {};
        int numberTokens = 0;
        for (int j = startCounter; j < tokensLength - 1; j++)
        {
                lastTokens[j-startCounter] = splitTokens[j];
                numberTokens++;
        }

        struct Process process = createProcess(lastTokens, numberTokens);
        processList[numberProcess] = process;
        numberProcess++;

        logicstics.numberProcesses = numberProcess;
        logicstics.outputCode = returnCode;
        return logicstics;
}

void ExecuteCommand(struct Process* processes, int numProcesses, int* status)
{

        pid_t pid[MAX_PROCESS];
        int fileDescriptors[2];  
        int previousFileDescriptor; 
        previousFileDescriptor = open("/dev/null", 0); 
        for (int i = 0; i < numProcesses; i++)
        {
                pipe(fileDescriptors);
                
                pid[i]= fork();
                 
                if (pid[i] == 0) {
                        
                        // If it is not the first process
                        if (i != 0) {
                                dup2(previousFileDescriptor, STDIN_FILENO);  
                        }
                        
                        // If it is not the last process
                        if (i != (numProcesses - 1)) {
                                dup2(fileDescriptors[1], STDOUT_FILENO);
                        }
                        // Close all the pipes 
                        close(previousFileDescriptor); 
                        close(fileDescriptors[0]);
                        close(fileDescriptors[1]);

                        if (processes[i].redirection)
                        {
                                int fd;
                                /* Opens the filename and redirects the stream to the file */
                                fd = open(processes[i].fileName, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                                dup2(fd, STDOUT_FILENO);
                                if (processes[i].errorRedirect)
                                        dup2(fd, STDERR_FILENO);
                                close(fd);
                        }
                        execvp(processes[i].args[0], processes[i].args);
                        fprintf(stderr, "Error: command not found\n");
                        exit(1);
                } else if (pid[i] > 0) {
                        close(fileDescriptors[1]);
                        close(previousFileDescriptor); 
                        previousFileDescriptor = fileDescriptors[0]; 
                } else {
                        // Handle Errors
                        perror("fork");
                        exit(1);
                }
        }
        close(fileDescriptors[0]); 
        for (int i = 0; i < numProcesses; i++)
        {
                waitpid(pid[i], &status[i], 0); 
        }
        
}
/*
1. Fork every process in the shell
Shell(Parent) -> P1(Child) 
Shell(Parent) -> P2(Child) ... 

2. Before you execute, you call dup2(pipe(read), STDIN), dup2(pipe(write), STDOUT), close both pipes. 
If the process is the first process, we don't want dup2 on the read pipe. 
If the process is the last process, we don't want dup2 on the write pipe. 

3. Execute The process. 

4. Repeat the loop 

5. If you are the shell, you are waiting for a process to finish. 
If the process finishes, we need to store it in the correct spot. 

*/

int main(void)
{
        char cmd[CMDLINE_MAX];

        while (1)
        {
                char *nl;
                int retval;

                /* Print prompt */
                printf("sshell$ ");
                fflush(stdout);

                /* Get command line */
                fgets(cmd, CMDLINE_MAX, stdin);

                /* Print command line if stdin is not provided by terminal */
                if (!isatty(STDIN_FILENO))
                {
                        printf("%s", cmd);
                        fflush(stdout);
                }

                /* Remove trailing newline from command line */
                nl = strchr(cmd, '\n');
                if (nl)
                        *nl = '\0';

                /* If user just presses enter, it continues to ask for input */
                if (!strcmp(cmd, ""))
                        continue;
                /* Create a process list full of process structures */
                struct Process processList[MAX_PROCESS];

                /* Copies the commandline for later use */
                char copycmd[CMDLINE_MAX];
                strcpy(copycmd, cmd);

                /* Parses the command line into processes */
                struct ProcessLogic logistics = ParseCommandLine(cmd, processList);
                if (logistics.outputCode)
                {
                        continue;
                }
                struct Process first_process = processList[0];

                /* Builtin command for exit*/
                if (!strcmp(first_process.args[0], "exit"))
                {
                        fprintf(stderr, "Bye...\n");
                        fprintf(stdout, "+ completed 'exit' [0]\n");
                        break;
                }

                /* Builtin command for cd */
                if (!strcmp(first_process.args[0], "cd"))
                {
                        retval = ExecuteCd(first_process.args[1]);
                        fprintf(stdout, "+ completed '%s' [%d]\n",
                                copycmd, retval);
                        continue;
                }

                /* Builtin command for pwd */
                if (!strcmp(first_process.args[0], "pwd"))
                {
                        ExecutePwd();
                        fprintf(stdout, "+ completed '%s' [0]\n", copycmd);
                        continue;
                }

                if (!strcmp(first_process.args[0], "sls"))
                {
                        ExecuteSls();
                        fprintf(stdout, "+ completed '%s' [0]\n", copycmd);
                        continue;
                }

                /* Regular command */
                int retvalues[MAX_PROCESS];
                ExecuteCommand(processList, logistics.numberProcesses, retvalues);
                fprintf(stdout, "+ completed '%s' [%d]\n",
                        copycmd, retvalues[0]);
        }

        return EXIT_SUCCESS;
}
// P1 -> P2 