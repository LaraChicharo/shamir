#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include "cipher.h"
#include "hasher.h"
#include "shamir.h"
#include "main.h"


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
 * Promts the user for a password to encrypt a document.
 * @param pass the variable where the password will be stored.
 * @param keysize a pointer to be filled with the size of the password.
 */
void get_password(char **pass, int *keysize) {
	int c;
	int i = 0;
	puts("Type password then INTRO");
	while((c = getchar()) != EOF && c != '\n') {
		(*pass)[i++] = c;
		if (i > MAX_PASS_LEN)
			error_message(1, "Password too long\n");
	}
	(*pass)[i] = '\0';
	*keysize = i;
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
	if (!(strlen(argv[1]) < MAX_LEN_FILENAME &&
				strlen(argv[5]) < MAX_LEN_FILENAME))
		error_message(1, "Filename too long.");
	int nshares = str_to_int(argv[3]);
	if (nshares <= 2)
		error_message(1, "Too few shares!");
	int min_shares = str_to_int(argv[4]);
	if (!(1 < min_shares && min_shares <= nshares))
		error_message(1, "Min shares value is wrong");
	validate_file(argv[5], "r", "Error reading file to encrypt");
}

/**
 * Gets the length of the name of the encrypted file.
 * Adds the name of the file to be encrypted and the extension
 * of the new file.
 * @param original_name name of the file to encrypt.
 * @returns the length of the name of the new encrypted file.
 * */
int get_namesize_encrypted_file(char* original_name) {
	return strlen(original_name) + strlen(ENCRYPTED_EXT) + 1;
}

/**
 * Merges the name of the file to encrpt and a file extension.
 * @param original name name of the file to encrypt.
 * @param encname array of chars long enough to store the name
 * of the new encrypted file.
 */
void get_name_encrypted_file(char* original_name, char** encname) {
	char newname_[get_namesize_encrypted_file(original_name)];

	int i = 0;
	while (original_name[i] != '\0') {
		newname_[i] = original_name[i];
		i++;
	}
	int j = 0;
	int len = strlen(ENCRYPTED_EXT);
	while (j < len)
		newname_[i+j] = ENCRYPTED_EXT[j++];
	newname_[i + j] = '\0';
	strcpy(*encname, newname_);
}

/**
 * Gets the size of the name of the decrypted file.
 * Substracts the lenght of the ext. to the name of the encrypted file.
 * @param encname filename of encrypted document.
 * @returns the size of the name of the decrypted file.
 */
int get_namesize_decrypted_file(char* encname) {
	return strlen(encname) - strlen(ENCRYPTED_EXT);
}

/**
 * Gets the name of the decrypted file to create it.
 * @param encname filename of encrypted document.
 * @param original name pointer to str to store the name in it.
 */
void get_name_decrypted_file(char* encname, char** original_name) {
	int i = 0;
	while (i < get_namesize_decrypted_file(encname))
		(*original_name)[i] = encname[i++];
}

/*
 * Validates the arguments when a user wants to decryp a file.
 * Exits with an error message if things are not right.
 * @param argv command line arguments.
 */
void validate_decipher_option(char** argv) {
	if (strlen(argv[2]) > MAX_LEN_FILENAME ||
			strlen(argv[3]) > MAX_LEN_FILENAME)
		error_message(1, "Filename too long.");
	validate_file(
			argv[2], "r", "Error reading file containing shares.");
	validate_file(argv[3], "r", "Error reading the encrypted file.");
}

int main(int argc, char** argv) {
	if (argc < 4)
		error_message(1, "Too few arguments");
	if (strcmp(argv[1], "c") == 0) {

		validate_cipher_option(argc, argv);

		FILE *saved_eval = fopen(argv[2], "w");
		int eval_number_required = str_to_int(argv[3]);
		int minimum_points_needed = str_to_int(argv[4]);

		char *pass = (char*)malloc(MAX_PASS_LEN * sizeof(char));
		int keysize;

		get_password(&pass, &keysize);

		char *buff = malloc((sizeof(char) * 32 * 8) + 1);
		hash_string_to_int(&argv[1], &buff);
		mpz_t secret;
		mpz_init2(secret, MPZ_LIMIT);
		mpz_set_str(secret, buff, 2);

		create_shares(
				eval_number_required, minimum_points_needed, secret, saved_eval);
		FILE *plainfp = fopen(argv[5], "r");
		char* encname = malloc(
				sizeof(char) * get_namesize_encrypted_file(argv[5]));
		get_name_encrypted_file(argv[5], &encname);
		FILE *encrfp = fopen(encname, "w");

		encrypt_(&plainfp, &encrfp, pass, keysize);

		printf("Your pass: %s\n", pass);
		free(pass);
		free(encname);
		fclose(plainfp);
		mpz_clear(secret);
		fclose(encrfp);
	} else if (strcmp(argv[1], "d") == 0) {
		validate_decipher_option(argv);

		FILE *read = fopen(argv[2], "r");
		int length = define_length(read);
		//if length = 1, i.e, there is only 1 point, an error ocurrs
		struct SHARE_* evals = malloc(sizeof(struct SHARE_) * length - 1);
		reader(read, &evals);
		int i,j;

		mpz_t * poly = malloc(length * sizeof(mpz_t));
		init_polynomial(&poly, length);	

		mpz_t **aux = malloc(length * sizeof(mpz_t*));
		fill_aux_polynomial(&aux, length);

		lagrange_basis(&evals, &aux, length);
		rebuild_polynomial(&poly, &aux, &evals, length);

		char *pass = (char*)malloc(MAX_PASS_LEN * sizeof(char));
		mpz_out_str(stdout, 10, poly[0]);
		mpz_get_str(pass, 10, poly[0]);
	


		char* original_name = malloc(
				sizeof(char) * get_namesize_decrypted_file(argv[3]));
		get_name_decrypted_file(argv[3], &original_name);
		// printf("original_name: %s\n", original_name);

		FILE *encrfp = fopen(argv[3], "r");
		FILE *decrfp = fopen("decrypted", "w");

		printf("pass:%s\n", pass);

		decrypt_(&encrfp, &decrfp, pass, strlen(pass));

		free(pass);
		free(original_name);
		fclose(encrfp);
		fclose(decrfp);
	} else {
		error_message(
				1,
				"The first arg must be 'c' to encrypt or 'd' to decrypt.");
	}
	return 0;
}
