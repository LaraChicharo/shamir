#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
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

/**
 * Prints an error message on stderr.
 * Procedes to exit the program reporting the given error code.
 * @param errcode error code.
 * @param msg message to display on stderr.
 */
void error_message(int errcode, char* msg) {
	fprintf(stderr, "%s\n", msg);
	exit(errcode);
}

/**
 * Converts a string to int.
 * Only works with strings that represent a number (integer).
 * Example: '5', '0', '1000'.
 * On error, the execution of the program stops.
 * @param str: string to be converted.
 * @returns the integer.
 */
long int str_to_int(char *str) {
	char* p;
	errno = 0;
	long int n = strtol(str, &p, 10);
	if (errno != 0)
		error_message(errno, strerror(errno));
	else if (*p != '\0')
		error_message(1, "Faulty string");
	else if (n > INT_MAX)
		error_message(1, "Number too large!");
	return n;
}

/**
 * Tries to open a file with the given mode.
 * Exits the program with an error message if the file cant be opened.
 * @param filename name of the file
 * @param mode mode in which the file will be opened
 * @param errmsg error message to display if the file cant be opened.
 * */
void validate_file(char* filename, char* mode, char* errmsg) {
	FILE* fp = fopen(filename, mode);
	if (fp == NULL)
		error_message(1, errmsg);
	fclose(fp);
}

/**
 * Validates the arguments when the user wants to encrypt a file.
 * Stops the execution if some parameter is off.
 * @param argc number of arguments passed to the program.
 * @param argv array of the arguments.
 */
void validate_cipher_option(int argc, char** argv) {
	if (argc != 6)
		error_message(1, "Not enough arguments!");
	int nshares = str_to_int(argv[3]);
	if (nshares <= 2)
		error_message(1, "Too few shares!");
	int min_shares = str_to_int(argv[4]);
	if (!(1 < min_shares && min_shares <= nshares))
		error_message(1, "Min shares value is wrong");
	validate_file(argv[5], "r", "Error reading file to encrypt");
}

int main(int argc, char** argv) {
	if (argc < 4)
		error_message(1, "Too few arguments");
	if (strcmp(argv[1], "c") == 0) {
				
		validate_cipher_option(argc, argv);

		char *pass = (char*)malloc(MAX_PASS_LEN * sizeof(char));
		int keysize;

		get_password(&pass, &keysize);

		FILE *plainfp = fopen(argv[5], "r");
		FILE *encrfp = fopen("encrypted.aes", "w");
		
		encrypt(&plainfp, &encrfp, pass, keysize);

		printf("Your pass: %s\n", pass);
		free(pass);
		fclose(plainfp);
		fclose(encrfp);
	} else if (strcmp(argv[1], "d") == 0) {
		char *pass = (char*)malloc(MAX_PASS_LEN * sizeof(char));
		int keysize;

		get_password(&pass, &keysize);

		FILE *encrfp = fopen(argv[2], "r");
		FILE *decrfp = fopen("decrypted.txt", "w");

		decrypt(&encrfp, &decrfp, pass, keysize);

		free(pass);
		fclose(encrfp);
		fclose(decrfp);
	} else {
		error_message(
			1,
			"The first arg must be 'c' to encrypt or 'd' to decrypt.");
	}
	return 0;
}
