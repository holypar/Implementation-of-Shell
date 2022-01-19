# SHELL #

## Introduction ##
For our project, we created a simple shell that executes basic commands on the terminal. It supports both output redirection as well as piping. 
## Implementation ##

#### Data Structure ####
In order to parse the command line correctly, we deided to use a data structure to store various properties related to a process. 
Things like redirection, arguements and filenames would be stored here. 
Each data structure would be in relation to one specific process on the command line. 

We also used another data structure to store the logistics of the data.
After each creation of parsing the command line, we store a process logistics.  
Specific things are stored like the total number of processes and the output code which we use for checking errors. 

#### Builtin Functions ####
For the built in functions, we execute both pwd and cd as specified using the syscalls from lecture. 
Each of them has a specific return value associated with it. We handled the errors based on the return value. 

#### Step 1: Splitting the command line ####
In order to parse the command line, we first split the command line into tokens by whitespace.
Additionally, edges cases are checked like if specific token (such as the '>' or '|') is inside the one token, we make sure to split it properly. 
Then afterwards, we split these properly and store it into an array of strings. 

#### Step 2: Parsing the command line ####
After splitting the command line, we go through and check for '|' tokens.
These indicate our command line should be split into multiple processes or not. 
For every new '|', we have an additional process. 
Then, we go through each part to check for a file redirection or file error redirection. 
Then, all of this data is put into a struct called process. 
This process is stored into an array of process structures. 
Therefore, if there are multiple processes, in the case of using pipes, we can account for that. 
Additionally, we store the number of processes and output code (Determine if there was a parsing error or not)

#### Step 2.5: Parsing Errors ####
To handle parsing errors, this is all done before we turn the commandline into processes.
To do this, we go through the command line after the splits and check each individual token is in the correct order as specified in the prompt. 
If there is an error, we return an output code of 1 to indicate a parsing error. 
This will prompt the shell to print a error message to stderr and exit from the parsing phase.
If the output code is a 1, this prompts the shell to not execute the commandline and move on to the next user input.  

#### Step 3: Builtin Functions ####
Before, we execute any function, we need to check if the first process specifies us to do a builtin function from the shell. 
Thus, if the first function is a builtin, we execute it based upon our specified code which gives us a return value. 
Otherwise, we move on. 

#### Step 4: Executing the processes ####
To execute all the processes, we need a array of structs of the processes, the number of processes, and somewhere to store the return values. 
To start, we make sure to fork once for every process. 
Afterwards, we create a pipe opening file descriptors for use if there are multiple processes. 
Additionally, we also use a dummy file descriptor used to store the previous file descriptors if there are multiple processes. 
This implementation was suggested by another student (Credit: Curtis Stofer). 
In this method, we use the dummy file descriptor to store the previous file descriptor for STDIN of the next process. 
Therefore, this file descriptor gets saved to something before moving onto next loop and not get closed beforehand. 

We loop through the processes and for the child processes, connect the STDIN of the process to the previous file descriptor read, and the STDOUT of the current file descriptors pipe.
In the child, we additionally check for error redirection and file redirections as well. 
For the parent (shell), we make sure to close all file descriptors not used and assign the previous file descriptor to the current one. 

After this process, we wait for all the children processes to finish and store the return values in an array 
Once, this process is done, we make sure to print the final message to the screen to STDOUT. 

## Conclusion ##

We finally did it.