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



int main(void)
{
	char * cmd, line[MAXLEN], * args[MAXNUM];
	int background, i, j, fd;
	int pid;
	char dir[1024];
	
	
	signal(SIGALRM, sighandler);
	signal(SIGINT, sighandler);
	
	while (1) {
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
			} else if (strcmp(args[j], "<") == 0) {
				printf("< catched\n");
			} else if (strcmp(args[j], "|") == 0) {
				printf("| catched\n");
			}
		}
		
		/* fork to run the command */
		switch (pid = fork()) {
			case -1:
				/* error */
				perror("fork");
				continue;
			case 0:
				/* child process */

				/* write output to file lul */
				fd = open("lul", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
				dup2(fd, 1);   // make stdout go to file
				close(fd);     // fd no longer needed - the dup'ed handles are sufficient

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
