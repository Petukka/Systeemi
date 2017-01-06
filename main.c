#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

#define LOGOUT 30
#define MAXNUM 40
#define MAXLEN 160

void sighandler(int sig)
{
	switch (sig) {
		case SIGALRM:
			printf("\nautologout\n");
			exit(0);
		case SIGINT:
			printf("\nuser interrupt\n");
			exit(0);
		default:
			break;
	}
	return;
}

void pipeHandler(char **args, char **pipeArgs) {
	/* fork to run the command */
	int pid, fd[2];

	pipe(fd);	
	switch (pid = fork()) {
		case -1:
			/* error */
			perror("fork");
			exit(1);
		case 0:
			/* child process */
			close(0);
			dup2(fd[0], 0);
			close(fd[1]);
			execvp(pipeArgs[0], pipeArgs);
			perror("execvp");
		default:
			/* parent */
			close(1);
			dup2(fd[1], 1);
			close(fd[0]);
			execvp(args[0], args);
			perror("execvp");
	}
}

int main(void)
{
	char * cmd, line[MAXLEN], * args[MAXNUM], **pipeArgs;
	int background, i, j, in, out, inout, p;
	int pid;
	char dir[1024], input[64], output[64];
	
	signal(SIGALRM, sighandler);
	signal(SIGINT, sighandler);
	
	while (1) {
		out = 0;
		in = 0;
		inout = 0;
		background = 0;

		getcwd(dir, sizeof(dir));
		
		/* print the prompt */
		printf("$ %s > ", dir);
		/* set the timeout for alarm signal (autologout) */
		/*alarm(LOGOUT);*/
		
		/* read the users command */
		if (fgets(line,MAXLEN,stdin) == NULL) {
			printf("\nlogout\n");
			exit(0);
		}
		line[strlen(line) - 1] = '\0';
		
		if (strlen(line) == 0)
			continue;
		
		/* start to background? */
		if (line[strlen(line)-1] == '&') {
			line[strlen(line)-1]=0;
			background = 1;
		}
		
		/* split the command line */
		i = 0;
		cmd = line;
		while ( (args[i] = strtok(cmd, " ")) != NULL) {
			printf("arg %d: %s\n", i, args[i]);
			i++;
			cmd = NULL;
		}


		
		if (strcmp(args[0],"exit")==0) {
			exit(0);
		}


		if (strcmp(args[0],"cd")==0) {
			if (args[1] == NULL) {
				chdir(getenv("HOME"));				
			} else {
				chdir(args[1]);
			}
			continue;
		}

		for (j = 0; j < i; j++) {
			if (strcmp(args[j], ">") == 0) {
				printf("> catched\n");
				args[j] = NULL;
				strcpy(output, args[j+1]);
				out = 1;
			} else if (strcmp(args[j], "<") == 0) {
				printf("< catched\n");
				args[j] = NULL;
				strcpy(input, args[j+1]);
				in = 1;
			} else if (strcmp(args[j], "|") == 0) {
				printf("| catched\n");
				p = 1;
				args[j] = NULL;
				pipeArgs = args + j + 1;
			}
		}

		if ((out == 1) && (in == 1)) {
			in = 0;
			out = 0;
			inout = 1;
		}
		
		/* fork to run the command */
		switch (pid = fork()) {
			case -1:
				/* error */
				perror("fork");
				continue;
			case 0:
				/* child process */

				if (pip == 1) {

				}
				
				if (inout == 1) {
					printf("kaksisuuntainen redirection\n");
					FILE* file = fopen(input, "r");
					dup2(fileno(file), 0);
					fclose(file);
					file = fopen(output, "w");
					dup2(fileno(file), 1);
					fclose(file);
				}
				else if (out == 1) {
					printf("output redirection\n");
					FILE* file = fopen(output, "w");
					dup2(fileno(file), 1);
					fclose(file);
				} else if (in == 1) {
					printf("input redirection\n");
					FILE* file = fopen(input, "r");
					dup2(fileno(file), 0);
					fclose(file);
				} else if (p == 1) {
					printf("pipe redirection\n");
					i = 0;
					while (args[i] != NULL) {
						printf("arg %d: %s\n", i, args[i]);
						i++;
					}
					i = 0;
					while (pipeArgs[i] != NULL) {
						printf("arg %d: %s\n", i, pipeArgs[i]);
						i++;
					}
					pipeHandler(args, pipeArgs);
					exit(1);
				}

				execvp(args[0], args);
				perror("execvp");
				exit(1);
			default:
				/* parent (shell) */
				if (!background) {
					alarm(0);
					//waitpid(pid, NULL, 0);
					while (wait(NULL)!=pid)
						printf("some other child process exited\n");
				}

				break;
		}
	}
	return 0;
}
