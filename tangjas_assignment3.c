// Program Name: tangjas_assignment3.c
// Author: Jason Tang
// Program creates small shell with specific requirements for built-in and
// other commands

// Adapted from URL: https://canvas.oregonstate.edu/courses/2007910/files/112284925
// sample_parser.c
// Adapted from Module 6 and 7 Explorations
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
int foreground_only_mode = 0;

void handle_sigint(int signo);
void handle_sigtstp(int signo);

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
            // child process

			// fill out int_action struct
			struct sigaction int_action = {0};
			int_action.sa_handler = SIG_IGN;
			sigaction(SIGINT, &int_action, NULL);

			// fill out dfl_action struct
			struct sigaction dfl_action = {0};
			dfl_action.sa_handler = SIG_DFL;
			sigaction(SIGINT, &dfl_action, NULL);

			// fill out tstp_action struct
			struct sigaction tstp_action = {0};
			tstp_action.sa_handler = handle_sigtstp;
			sigfillset(&tstp_action.sa_mask);
			tstp_action.sa_flags = SA_RESTART;
			sigaction(SIGTSTP, &tstp_action, NULL);

			if (command->is_bg && foreground_only_mode == 0) {
				sigaction(SIGINT, &int_action, NULL);
				sigaction(SIGTSTP, &tstp_action, NULL);
			} else {
				sigaction(SIGINT, &dfl_action, NULL);
				sigaction(SIGTSTP, &tstp_action, NULL);
			}

			// TODO: handle input/output

			// execute process
			execvp(command->argv[0], command->argv);

			// fork() failed
			perror("execvp");
			exit(EXIT_FAILURE);
			break;

        default:
            // parent process
			int child_status;

			// if (command->is_bg && foreground_only_mode == 0) {
			// 	getppid()
			// } else {

			// }
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


void handle_sigint(int signo) {
	char* message = "Caught SIGINT, sleeping for 10 seconds\n";
	// We are using write rather than printf
	write(STDOUT_FILENO, message, 39);
	sleep(10);
}


void handle_sigtstp(int signo) {
    if (foreground_only_mode == 0) {
        char* msg = "\nEntering foreground-only mode (& is now ignored)\n: ";
        write(STDOUT_FILENO, msg, strlen(msg));
        foreground_only_mode = 1;
    } else {
        char* msg = "\nExiting foreground-only mode\n: ";
        write(STDOUT_FILENO, msg, strlen(msg));
        foreground_only_mode = 0;
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