#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dirent.h>

#define PATH_MAX 4096
// Found within https://stackoverflow.com/questions/9449241/where-is-path-max-defined-in-linux

#define CMDLINE_MAX 512

void executeCd(char* changeTo){
        
        // Check if the path exists first 
        DIR *directoryPointer; 
        struct dirent *dp; 
        
        directoryPointer = opendir("."); 

        while ((dp = readdir(directoryPointer)) != NULL) {
                // If the path exists, 
                if (strcmp(dp->d_name, changeTo)) {
                        chdir(changeTo);
                        fprintf(stdout, "%s\n", "Changed Directory"); 

                } else {
                        fprintf(stderr, "%s\n", "Error: cannot cd into directory");
                }
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

void parseCommandLine(char* command, char** args) {
        
        
        int tokenInteger = 0; 
        
        char* token = strtok(command, " "); 
        
        while (token != NULL) {
                args[tokenInteger] = token; 
                tokenInteger++;
                token = strtok(NULL, " ");
        }
        args[tokenInteger] = NULL;
        
}

int ExecuteCommand(char* command) {
        
        pid_t pid;
        char* args[17]; 
        parseCommandLine(command, args); 

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

                /* Regular command */
                retval = ExecuteCommand(cmd);
                fprintf(stdout, "Return status value for '%s': %d\n",
                         cmd, retval);
        }

        return EXIT_SUCCESS;
}


