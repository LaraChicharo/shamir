#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cipher.h"

#define MAX_PASS_LEN 3


void get_password(char **pass, int *keysize) {
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
	*keysize = i;
}


int main(int argc, char** argv) {
	if (strcmp(argv[1], "c") == 0) {
		// not catching errors yet

		char *pass = (char*)malloc(MAX_PASS_LEN * sizeof(char));
		int keysize;
		
		get_password(&pass, &keysize);
		
		FILE *plainfp = fopen(argv[2], "r");
		FILE *encrfp = fopen("encrypted.aes", "w");
		
		encrypt(&plainfp, &encrfp, pass, keysize);
		
		printf("Your pass: %s\n", pass);
		free(pass);
		fclose(plainfp);
		fclose(encrfp);
	} else if (strcmp(argv[1], "d") == 0) {
		// decipher
	} else {
		// bad input
	}
	return 0;
}
