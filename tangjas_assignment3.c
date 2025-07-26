// Program Name: tangjas_assignment3.c
// Author: Jason Tang
// Program creates CLI-based UI to print movies depending on option selected:
// 1 for movies released in specified year, 2 for highest rated movie each year
// 3 for movies in a specific lanugage, 4 to exit

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

#define INPUT_LENGTH 2048
#define MAX_ARGS 512


struct command_line
{
	char *argv[MAX_ARGS + 1];
	int argc;
	char *input_file;
	char *output_file;
	bool is_bg;
};


struct command_line *parse_input()
{
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


// Adapted from following Explorations:
// Process API - Monitoring Child Processes
// Process API - Executing a New Program
void execute_command(struct command_line *command) {
    execvp(command->argv[0], command->argv);

    // execvp returns only on error 
    perror("execvp");   
    exit(EXIT_FAILURE);

    pid_t spawnpid = -5;
    int intVal = 10;
    // If fork is successful, the value of spawnpid will be 0 in the child
    // and will be the child's pid in the parent
    spawnpid = fork();
    switch (spawnpid){
        case -1:
            perror("fork() failed!");
            exit(1);
            break;
        case 0:
            // spawnpid is 0 in the child
            intVal = intVal + 1;
            printf("I am the child! intVal = %d\n", intVal);
            break;
        default:
            // spawnpid is the pid of the child
            intVal = intVal - 1;
            printf("I am the parent! intVal = %d\n", intVal);
            break;
    }
}


int main()
{
	struct command_line *curr_command;

	while(true)
	{
		curr_command = parse_input();

        execute_command(curr_command);
	}
	return EXIT_SUCCESS;
}