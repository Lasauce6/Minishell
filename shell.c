#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>

int main() {
	while(1) {
		printf("$ ");
		char *buf = malloc(1024);
		scanf("%s", buf);

		if (strcmp(buf, "exit") == 0 || feof(stdin)) {
			exit(0);
		}

		if (fork() == 0) {	
			execlp(buf, buf, NULL);
		} else {
			wait(NULL);
		}

		free(buf);
	}
}
