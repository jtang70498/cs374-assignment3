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
#include <fcntl.h>

#define INPUT_LENGTH 2048
#define MAX_ARGS 512

int status = 0;
int foreground_only_mode = 0;

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
    // forking processs
    pid_t spawnpid = fork();

    switch (spawnpid) {

		// error case
        case -1:
			// fork() failed
            perror("fork() failed!");
            exit(EXIT_FAILURE);
            break;

		// child process
        case 0:
			// fill out sigint_action struct
			struct sigaction sigint_action = {0};
			sigint_action.sa_handler = SIG_DFL;
			sigfillset(&sigint_action.sa_mask);
			sigint_action.sa_flags = 0;
			sigaction(SIGINT, &sigint_action, NULL);

			// fill out sigtstp_action struct
			struct sigaction sigtstp_action = {0};
			sigtstp_action.sa_handler = handle_sigtstp;
			sigfillset(&sigtstp_action.sa_mask);
			sigtstp_action.sa_flags = SA_RESTART;
			sigaction(SIGTSTP, &sigtstp_action, NULL);

			if (command->is_bg && foreground_only_mode == 0) {
				sigaction(SIGINT, &sigint_action, NULL);
				sigaction(SIGTSTP, &sigint_action, NULL);
			}
			
			// for input redirection
			if (command->input_file != NULL) {
				// open source file
				int source_fd = open(command->input_file, O_RDONLY);
				if (source_fd == -1) {
					printf("cannot open %s for input\n", command->input_file);
					fflush(stdout);
					exit(1); 
				}
				// redirect stdin to source file
				dup2(source_fd, 0);
			}
			
			// for output redirection
			if (command->output_file != NULL) {
				// open target file
				int target_fd = open(command->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
				if (target_fd == -1) {
					printf("cannot open %s for output\n", command->output_file);
					fflush(stdout);
					exit(1);
				}
				// redirect stdout to target file
				dup2(target_fd, 1);
			}

			// for /dev/null redirection
			if (command->is_bg && foreground_only_mode == 0) {
				if (command->input_file == NULL) {
					// redirect stdin to /dev/null
					int null_fd = open("/dev/null", O_RDONLY);
					dup2(null_fd, 0);
				}
				if (command->output_file == NULL) {
					// redirect stdout to /dev/null
					int null_fd = open("/dev/null", O_WRONLY);
					dup2(null_fd, 1);
				}
			}

			// execute process
			execvp(command->argv[0], command->argv);

			// fork() failed
			printf("%s: no such file or directory\n", command->argv[0]);
			exit(EXIT_FAILURE);
			break;

		// parent process
        default:
			int child_status;
			
			// background command
			if (command->is_bg && foreground_only_mode == 0) {
				printf("background pid is %d\n", spawnpid);
				fflush(stdout);
			// foreground command
			} else {
				waitpid(spawnpid, &child_status, 0);
				if (WIFEXITED(child_status)) {
					status = WEXITSTATUS(child_status);
				} else {
					status = WTERMSIG(child_status);
					printf("terminated by signal %d\n", status);
					fflush(stdout);
				}
			}
            break;
    }
}


void handle_sigtstp(int signo) {
    if (foreground_only_mode == 0) {
        char* message = "\nEntering foreground-only mode (& is now ignored)\n: ";
        write(STDOUT_FILENO, message, 53);
        foreground_only_mode = 1;
    } else {
        char* message = "\nExiting foreground-only mode\n: ";
        write(STDOUT_FILENO, message, 33);
        foreground_only_mode = 0;
    }
}


int main() {
	int child_status;
	struct command_line *curr_command;

	// fill out sigint_action struct
	struct sigaction sigint_action = {0};
	sigint_action.sa_handler = SIG_IGN;
	sigfillset(&sigint_action.sa_mask);
	sigint_action.sa_flags = 0;
	sigaction(SIGINT, &sigint_action, NULL);

	// fill out sigtstp_action struct
	struct sigaction sigtstp_action = {0};
	sigtstp_action.sa_handler = handle_sigtstp;
	sigfillset(&sigtstp_action.sa_mask);
	sigtstp_action.sa_flags = SA_RESTART;
	sigaction(SIGTSTP, &sigtstp_action, NULL);

	while(true) {
		pid_t background_pid = waitpid(-1, &child_status, WNOHANG);
		if (background_pid > 0) {
			if (WIFEXITED(child_status)) {
				// print background process exit info
				printf("background pid %d is done: exit value %d\n", background_pid, WEXITSTATUS(child_status));
				fflush(stdout);
			} else if (WIFSIGNALED(child_status)) {
				// print background process termination by signal
				printf("background pid %d is done: terminated by signal %d\n", background_pid, WTERMSIG(child_status));
				fflush(stdout);
			}
		} else {
			break;
		}
	}

	while(true) {
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