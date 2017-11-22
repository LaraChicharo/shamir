#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cipher.h"

#define MAX_PASS_LEN 3


int get_password(char **pass) {
	int c;
	int i = 0;
	puts("Type password then INTRO");
	while((c = getchar()) != EOF && c != '\n') {
		(*pass)[i++] = c;
		if (i > MAX_PASS_LEN) {
			fputs("Password too long\n", stderr);
			exit(1);
		}
	}
	(*pass)[i] = '\0';
}


int main(int argc, char** argv) {
	if (strcmp(argv[1], "c") == 0) {
		// validate data
		// FILE *plainfp = fopen(argv[2]);
		char *pass = (char*)malloc(MAX_PASS_LEN * sizeof(char));
		get_password(&pass);
		printf("Your pass: %s\n", pass);
		// encrypt();
		free(pass);
	} else if (strcmp(argv[1], "d") == 0) {
		// decipher
	} else {
		// bad input
	}
}
