// Program Name: tangjas_assignment3.c
// Author: Jason Tang
// Program creates small shell with specific requirements for built-in and
// other commands

// Code adapted from 
// Title: SMALLSH
// Date: 7/26/25
// Adapted from URL: https://canvas.oregonstate.edu/courses/2007910/files/112284925
// sample_parser.c
// Author: CS 374 Instructors

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#define INPUT_LENGTH 2048
#define MAX_ARGS 512

int status = 0;


struct command_line {
	char *argv[MAX_ARGS + 1];
	int argc;
	char *input_file;
	char *output_file;
	bool is_bg;
};


struct command_line *parse_input() {
	char input[INPUT_LENGTH];
	struct command_line *curr_command = (struct command_line *) calloc(1, sizeof(struct command_line));

	// Get input
	printf(": ");
	fflush(stdout);
	fgets(input, INPUT_LENGTH, stdin);

	// Tokenize the input
	char *token = strtok(input, " \n");
	while(token){
		if(!strcmp(token,"<")){
			curr_command->input_file = strdup(strtok(NULL," \n"));
		} else if(!strcmp(token,">")){
			curr_command->output_file = strdup(strtok(NULL," \n"));
		} else if(!strcmp(token,"&")){
			curr_command->is_bg = true;
		} else{
			curr_command->argv[curr_command->argc++] = strdup(token);
		}
		token=strtok(NULL," \n");
	}
	return curr_command;
}


// Code adapted from following Explorations:
// Process API - Monitoring Child Processes
// Process API - Executing a New Program
void execute_command(struct command_line *command) {
    // if fork is successful, the value of spawnpid will be 0 in the child
    // and will be the child's pid in the parent
    pid_t spawnpid = fork();
    switch (spawnpid) {
        case -1:
			// fork() failed
            perror("fork() failed!");
            exit(EXIT_FAILURE);
            break;
        case 0:
            // child process, replace with new command
            execvp(command->argv[0], command->argv);
			perror("execvp");   
			exit(EXIT_FAILURE);
			break;
        default:
            // parent process, foreground
			int child_status;
			waitpid(spawnpid, &child_status, 0);
			if (WIFEXITED(status)) {
				status = WEXITSTATUS(child_status);
			} else {
				status = WTERMSIG(child_status);
				printf("terminated by signal %d\n", status);
    			fflush(stdout);
			}
            break;
    }
}


int main() {
	struct command_line *curr_command;

	while(true)
	{
		curr_command = parse_input();
		
		// commands for 'exit', 'cd', 'status'
        if (strcmp(curr_command->argv[0], "exit") == 0) {
			exit(0);
		} else if (strcmp(curr_command->argv[0], "cd") == 0) {
			chdir(curr_command->argv[1]);
			continue;
		} else if (strcmp(curr_command->argv[0], "status") == 0) {
			printf("exit value %d\n", status);
			continue;
		} else {
			execute_command(curr_command);
		}
	}
	return EXIT_SUCCESS;
}