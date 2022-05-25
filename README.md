# ECS 150 Project 1: Implementation of Shell #
*by Peng Xiao and Parminder Singh*
## **Introduction** ##
For our project, we created a simple shell that executes basic commands on the terminal. It supports both output redirection as well as piping. To properly implement our project, we make use of data structures recommended by the professor to store various things about a process. We also make sure the properly implement parsing of the command line errors and output correctly to specific output streams. 
## **Implementation** ##

### **Data Structure** ###
In order to parse the command line correctly, we deided to use a data structure to store various properties related to a process. 
Things like redirection, error redirections, arguements and filenames would be stored here. 
Each data structure would be in relation to one specific process on the command line. 

What is stored: 
    
1. args: Arguments (including the command) to put into the exec function 
2. redirection: A redirection boolean indicating if there is a file redirection. (true if there is file redirection)
3. errorRedirection: A boolean indicating if there is a stderr redirection to the next process or file. (true if there is error redirection)
4. filename: An associated filename if there is a file redirection. Otherwise it defaults to an empty string. 

We also used another data structure to store the logistics of the data.
After each creation of parsing the command line, we store a process logistics.  
Specific things are stored like the total number of processes and the output code which we use for checking errors. 

What is stored: 
    
1. outputCode: An output code indicating whether if there is an error when parsing the command line. (1 indicates an error, 0 indicates no errors)
2. numProcesses: The total number of processes after parsing the command line. 

### **Builtin Functions** ###
For the built in functions, we execute both pwd and cd as specified using the syscalls from lecture. 
Each of them has a specific return value associated with it. We handled the errors based on the return value that they give. 

We implemented functions as *pwd*, *cd*, and *sls* from as built in functions to use.  

---------------------------

### **Step 1: Splitting the command line** ###
In order to parse the command line, we first split the command line into tokens by whitespace.
This is done using functions such as strtok(). 

However, we need to also check for edge cases such as what if the special symbol ('>' or '|') is inside the token itself. 
Therefore, edges cases are checked like if specific symbol (such as the '>', '|', '>&', or '|&') is inside the one token, we make sure to split it properly. 
To do this, we split it by the string before the symbol, the symbol, and the string after the symbol and store them all separatly. 

Otherwise, if it is just a normal token, we store it properly. 
Afterwards, an array of strings are returned with all the tokens separated properly.  

---------------------------

### **Step 2: Parsing the command line** ###
Before, we do anything, we must check for any parsing errors. 
To do this, we first split the command line into tokens, then we push the tokens to a function called CheckParsing which handles all parsing errors. (See Step 2.1) 

After checking for errors from the command line, we go through and check for '|' tokens.
These indicate our command line should be split into multiple processes or not. 
For every new '|', we have an additional process. 
If we notice a '|' symbol, we take all tokens before it and launch a function to create a process struct (See Step 2.2). If there is a '&' after the symbol, we need to update the process error redirection attribute as well. i

Then, all of this data is put into a struct called process. 
This process is then stored into an array of process structures. 
Therefore, if there are multiple processes, in the case of using pipes, we can account for that. 
Additionally, we store the number of processes and output code (to determine if there was a parsing error or not). 

---------------------------

#### **Step 2.1: Parsing Errors** ####
To handle parsing errors, this is all done before we turn the commandline into processes.
To do this, we go through the command line after the splitting it and check each individual token is in the correct order as specified in the prompt.

Basically, we go through the all the tokens one by one and check against various cases. Most of the time, we are just comparing the current token with the next token in the list and checking that they are. 
If there is an error, we return an output code of 1 to indicate a parsing error. 
This will prompt the shell to print a error message to stderr and exit from the parsing phase.
If the output code is a 1, this prompts the shell to not execute the commandline and move on to the next user input.  

---------------------------

#### **Step 2.2: Create a process** ####
To create a process, we take all the tokens related to the process and store it into a struct. 
We make sure to check every token for a potential symbol of '>'. 
If we reach a '>' symbol, it means we are doing a redirection process to a file, and thus update the redirection property for the process struct. 

We make sure to store the filename after this symbol as well. 
Additionally, if there is an '&' after the symbol, we make sure to update the error redirection attribute for the process as well. 

---------------------------

### **Step 3: Builtin Functions** ###
Before, we execute any function, we need to check if the first process specifies us to do a builtin function from the shell. 
To do this, we just check the first args of the first process matches a specific builtin call. 
If the first function is a builtin, we execute it based upon our specified code which gives us a return value. 
Otherwise, we move on. 

---------------------------

### **Step 4: Executing the processes** ###
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

---------------------------

## **Conclusion** ##
After completing the shell we learned a variety of topics discussed in lecture. We learned how a simple command is ran without using system call. We also learned what works "behind the scenes" of shell such as working with multiple processes using piping. We also became a lot better at parsing data using functions that we were not too familiar with before. The hardest aspect of this project for us was implementing piping because it was harder to debug because we couldn't go into the child process in gdb.
