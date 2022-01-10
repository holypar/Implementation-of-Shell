#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#define CMDLINE_MAX 512


// Make a function to parse the command line 
// Input: command line string 
// Output: array of substrings from command line 

//**char an array of strings

// Function for executing a comand 
// Source: Lecture from fork_exec_wait.c
// Input: User inputted command
// Output: exit status of execution 
 

int ExecuteCommand(char* command) {
        
        pid_t pid;
        
        pid = fork();
        
        char* args[] = {};
        int tokenInteger = 0; 
        
        char* token = strtok(command, " "); // "date" gets saved to token
        strcpy(args[tokenInteger], token); //store in args 
        //date -u -v 
        while (token != NULL) {
                tokenInteger++;
                token = strtok(NULL, " ");
                strcpy(args[tokenInteger], token);
        }
        tokenInteger++;
        args[tokenInteger] = NULL; 

        if (pid == 0) {
                // Child 
                execvp(args[0], args); // ,date, date -u -v
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

                /* Regular command */
                 retval = ExecuteCommand(cmd);
                 fprintf(stdout, "Return status value for '%s': %d\n",
                         cmd, retval);
        }

        return EXIT_SUCCESS;
}


