#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

volatile sig_atomic_t sigint_received = 0;

void sigint_handler(int signum) {
	sigint_received = 1;
}

int parse_line(char *s, char ***argv) {
	*argv = (char **)malloc(sizeof(char *));
	int count = 0;

	char *token = strtok(s, " \t\n");
	while (token != NULL) {
		(*argv)[count] = (char *)malloc(strlen(token) + 1);
		strcpy((*argv)[count], token);
		count++;

		*argv = (char **)realloc(*argv, (count + 1) * sizeof(char *));

		token = strtok(NULL, " \t\n");
	}

	(*argv)[count] = NULL;

	return count;
}

int main() {
	struct sigaction sa;
	sa.sa_handler = sigint_handler;
	sigaction(SIGINT, &sa, NULL);

	while (1) {
		printf("$ ");
		char *buf = malloc(1024);
		scanf(" %[^\n]", buf);

		if (strcmp(buf, "exit") == 0 || feof(stdin)) {
			exit(0);
		}

		char **argv;
		int argc = parse_line(buf, &argv);

		char *output_file = NULL;
		int saved_stdout = dup(STDOUT_FILENO);

		sigset_t mask, oldmask;
		sigemptyset(&mask);
		sigaddset(&mask, SIGINT);
		sigprocmask(SIG_BLOCK, &mask, &oldmask);

		if (argc > 2) {
			if (strcmp(argv[argc - 2], ">") == 0) {
				output_file = strdup(argv[argc - 1]);
			}
		}

		if (argc > 1 && strcmp(argv[argc - 1], "|") == 0) {
			free(argv[argc - 1]);
			argv[argc - 1] = NULL;
			int pipefd[2];
			if (pipe(pipefd) == -1) {
				perror("Erreur lors de la création du tube");
				exit(EXIT_FAILURE);
			}

			if (fork() == 0) {
				// enfant (première commande)
				close(pipefd[0]);
				dup2(pipefd[1], STDOUT_FILENO);
				close(pipefd[1]);

				struct sigaction sa_child;
				sa_child.sa_handler = SIG_DFL;
				sigaction(SIGINT, &sa_child, NULL);

				execvp(argv[0], argv);
				perror("Erreur dans execvp");
				exit(EXIT_FAILURE);
			} else {
				// parent
				wait(NULL);

				close(pipefd[1]);

				sigprocmask(SIG_SETMASK, &oldmask, NULL);

				char *buf2 = malloc(1024);
				scanf(" %[^\n]", buf2);

				char **argv2;
				int argc2 = parse_line(buf2, &argv2);
				free(buf2);

				if (fork() == 0) {
					// enfant (deuxième commande)
					close(pipefd[1]);

					dup2(pipefd[0], STDIN_FILENO);
					close(pipefd[0]);

					struct sigaction sa_child;
					sa_child.sa_handler = SIG_DFL;
					sigaction(SIGINT, &sa_child, NULL);

					execvp(argv2[0], argv2);
					perror("Erreur dans execvp");
					exit(EXIT_FAILURE);
				} else {
					// parent
					wait(NULL);

					close(pipefd[0]);

					sigprocmask(SIG_SETMASK, &oldmask, NULL);
				}

				for (int i = 0; i < argc2; i++) {
					free(argv2[i]);
				}
				free(argv2);
			}
		} else {
			if (output_file != NULL) {
				int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
				dup2(fd, STDOUT_FILENO);
				argv[argc - 2] = NULL;
				close(fd);
			}

			if (fork() == 0) {
				// enfant
				struct sigaction sa_child;
				sa_child.sa_handler = SIG_DFL;
				sigaction(SIGINT, &sa_child, NULL);

				execvp(argv[0], argv);
			} else {
				// parent
				wait(NULL);

				sigprocmask(SIG_SETMASK, &oldmask, NULL);
			}

			if (output_file != NULL) {
				dup2(saved_stdout, STDOUT_FILENO);
				free(output_file);
			}

			for (int i = 0; i < argc; i++) {
				free(argv[i]);
			}
			free(argv);
		}

		free(buf);
	}
}
