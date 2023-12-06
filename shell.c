#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>

// int main() {
// 	while(1) {
// 		printf("$ ");
// 		char *buf = malloc(1024);
// 		scanf("%s", buf);
//
// 		if (strcmp(buf, "exit") == 0 || feof(stdin)) {
// 			exit(0);
// 		}
//
// 		if (fork() == 0) {	
// 			execlp(buf, buf, NULL);
// 		} else {
// 			wait(NULL);
// 		}
//
// 		free(buf);
// 	}
// }
//

int main() {
	printf("$ ");
	char *buf = malloc(1024);
	scanf("%s", buf);
	char *argv[1024];
	int argc = parse_line(buf, argv);

	free(buf);


}

int parse_line(char *s, char **argv []) {
	// use strpbrk to find the first occurence of a character in a string
	int i = 0;
	char *p = strpbrk(s, " ");
	while (p != NULL) {
		argv[i] = s;
		i++;
		s = p + 1;
		p = strpbrk(s, " ");
	}
	argv[i] = NULL;
	return i;
}
