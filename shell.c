#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

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
	while (1) {
		printf("$ ");
		char *buf = malloc(1024);
		scanf(" %[^\n]", buf);

		if (strcmp(buf, "exit") == 0 || feof(stdin)) {
			exit(0);
		}

		char **argv;
		int argc = parse_line(buf, &argv);

		if (fork() == 0) {
			// enfant
			execvp(argv[0], argv);
			perror("Erreur dans execvp");
			exit(EXIT_FAILURE);
		} else {
			// parent
			wait(NULL);
		}

		for (int i = 0; i < argc; i++) {
			free(argv[i]);
		}
		free(argv);
		free(buf);
	}
}
