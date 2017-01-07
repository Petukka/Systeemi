/**
* CT30A3370 Käyttöjärjestelmät ja systeemiohjelmointi
* Systeemiohjelmointi-osan harjoitustyö
* 
* Tekijät:
* Petri Rämö, opiskelijanumero: 0438578
* Ilari Sahi, opiskelijanumero: 0438594
*
* 7.1.2017
*/

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

/* handles signals (user interrupt) */
void sighandler(int sig) {
	char dir[1024];
	switch (sig) {
		case SIGINT:			
			break;
		default:
			break;
	}

	getcwd(dir, sizeof(dir));
			
	/* print the prompt */
	printf("\n$ %s > ", dir);
	fflush(stdout);
	return;
}

/* handles piping */
void pipeHandler(char **args, char **pipeArgs) {	
	int fd[2];
	pid_t pid1;
	pid_t pid2;
	
	/* fork to run the command */
	switch (pid1 = fork()) {
		case -1:
			/* error */
			perror("fork");
			return;
		case 0:
			pipe(fd);
			/* create child and parent processes */
			switch(pid2 = fork()) {
				case -1:
					perror("fork");
					return;
				case 0:
					/* set output */
					dup2(fd[1], 1);
					close(fd[0]);
					execvp(args[0], args);
					perror("execvp");
					break;
				default:
					/* get input */
					waitpid(-1, NULL, 0);
					dup2(fd[0], 0);
					close(fd[1]);
					execvp(pipeArgs[0], pipeArgs);			
					perror("execvp");
			}
		default:
			/* wait for child (piping) to finish */
			waitpid(-1, NULL, 0);
	}
}


int main(void) {
	char * cmd, line[MAXLEN], * args[MAXNUM], **pipeArgs;
	int background, i, j, in, out, inout, p;
	int pid;
	char dir[1024], input[64], output[64];
	
	signal(SIGINT, sighandler);	
	
	/* main loop for terminal */
	while (1) {
		out = 0;
		in = 0;
		inout = 0;
		p = 0;
		background = 0;

		/* print the prompt */
		getcwd(dir, sizeof(dir));		
		printf("$ %s > ", dir);		
		
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
			i++;
			cmd = NULL;
		}
		
		/* test for exit */
		if (strcmp(args[0],"exit")==0) {
			exit(0);
		}

		/* test for cd (go home) only */
		if (strcmp(args[0],"cd")==0) {
			if (args[1] == NULL) {
				chdir(getenv("HOME"));				
			} else {
				chdir(args[1]);
			}
			continue;
		}

		/* test for redirections and pipe */
		for (j = 0; j < i; j++) {
			if (strcmp(args[j], ">") == 0) {
				args[j] = NULL;
				strcpy(output, args[j+1]);
				out = 1;
			} else if (strcmp(args[j], "<") == 0) {
				args[j] = NULL;
				strcpy(input, args[j+1]);
				in = 1;
			} else if (strcmp(args[j], "|") == 0) {
				p = 1;
				args[j] = NULL;
				pipeArgs = args + j + 1;
			}
		}

		/* test for multiple redirections */
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
				/* double redirection */
				if (inout == 1) {
					FILE* file = fopen(input, "r");
					dup2(fileno(file), 0);
					fclose(file);
					file = fopen(output, "w");
					dup2(fileno(file), 1);
					fclose(file);
				}
				/* output redirection */
				else if (out == 1) {
					FILE* file = fopen(output, "w");
					dup2(fileno(file), 1);
					fclose(file);
				}
				/* input redirection */
				else if (in == 1) {
					FILE* file = fopen(input, "r");
					dup2(fileno(file), 0);
					fclose(file);
				}
				/* pipe */
				else if (p == 1) {
					pipeHandler(args, pipeArgs);
					exit(1);
				}

				/* execute if not pipe */
				execvp(args[0], args);
				perror("execvp");
				exit(1);
			default:
				/* parent (shell) */
				if (!background) {
					alarm(0);
					/* wait for children to finish */
					while (wait(NULL)!=pid)
						printf("some other child process exited\n");
				}

				break;
		}
	}
	return 0;
}
