all: sshell 

sshell: sshell.c 
	gcc -g -Wall -Werror -Wextra -o sshell sshell.c 

clean: 
	rm -f sshell 