#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//I need include from here
#include <x86_64-linux-gnu/gmp.h>
#include "cipher.c"
#include "hasher.c"
#include "shamir.c"

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

		FILE *saved_eval = fopen(argv[2], "w");
		int eval_number_required = atoi(argv[3]);
		int minimum_points_needed = atoi(argv[4]);


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
		FILE *encrfp = fopen("encrypted.aes", "w");
		
		encrypt(&plainfp, &encrfp, pass, keysize);
		
		printf("Your pass: %s\n", pass);
		free(pass);
		fclose(plainfp);
		fclose(encrfp);
	}else if (strcmp(argv[1], "d") == 0) {
		/*char *pass = (char*)malloc(MAX_PASS_LEN * sizeof(char));
		int keysize;
		
		get_password(&pass, &keysize);*/

		FILE *read = fopen(argv[2], "r");
		int length = define_length(read);
		//if length = 1, i.e, there is only 1 point, an error ocurrs
		struct SHARE_* evals = malloc(sizeof(struct SHARE_) * length-1);
		reader(read, &evals);
		int i,j;
		mpz_t * poly = malloc(length * sizeof(mpz_t));
		init_polynomial(&poly, length);	
		
		mpz_t **aux = malloc(length * sizeof(mpz_t*));
		fill_aux_polynomial(&aux, length);
		
		lagrange_basis(&evals, &aux, length);
		rebuild_polynomial(&poly, &aux, &evals, length);

		char *pass = (char*)malloc(MAX_PASS_LEN * sizeof(char));
		mpz_get_str(pass, 10, poly[0]);

		FILE *encrfp = fopen(argv[3], "r");
		FILE *decrfp = fopen("decrypted.txt", "w");
		
		//keysize?
		decrypt(&encrfp, &decrfp, pass, length);

		free(pass);
		fclose(encrfp);
		fclose(decrfp);
	} else {
		// bad input
	}
	return 0;
}
