# SHELL #

## Introduction ##
For our project, we created a simple shell that executes basic commands on the terminal. It supports both output redirection as well as piping. 
## Implementation ##

#### Data Structure ####
In order to parse the command line correctly, we deided to use a data structure to store various properties related to a process. 
Things like redirection, arguements and filenames would be stored here. 
Each data structure would be in relation to one specific process on the command line. 
#### Parsing the command line ####
In order to parse the command line, we first split the command line into tokens, 
then separated tokens by a pipe to make distinct parts. 
Then we go through each part to check for a redirection and make a process struct. 
This process is stored into an array of process structures. 

#### Builtin Functions ####
For the built in functions, we execute both pwd and cd as specified using the syscalls from lecture. Each of them has a specific return value associated with it.

## Conclusion ##


We finally did it.