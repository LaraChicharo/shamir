#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "minunit.h"
#include "../cipher.h"

#define PLAINTEXT_NAME "src/test/zimmermann.txt"
#define ENCRYPTED_NAME "src/test/supersecret.aes"
#define DECRYPTED_NAME "src/test/decrypted"

#define KEY "00000000"  // https://www.quora.com/What-are-the-most-famous-computer-passwords-in-history/answer/Jeff-Nelson-32?srid=45st


int tests_run = 0;

int are_files_equal(FILE **a, FILE **b) {
	int c;
	while ((c = getc(*a)) != EOF) {
		if (c != getc(*b))
			return 0;
	}
	return 1;
}

char* test_size_encfile(void) {
	FILE *encfp = fopen(ENCRYPTED_NAME, "r");
	
	fseek(encfp, 0, SEEK_END); // seek to end of file
	int size = ftell(encfp);
	fclose(encfp);
	mu_assert("Encrypted file is empty", size > 0);
	return 0;
}

static char* test_encrypt_decrypt(void) {
	remove(ENCRYPTED_NAME);
	FILE *plainfp = fopen(PLAINTEXT_NAME, "r");
	FILE *encfp = fopen(ENCRYPTED_NAME, "w");

	int keylen = strlen(KEY);
	encrypt_(&plainfp, &encfp, KEY, keylen);
	fclose(encfp);

	char *res = test_size_encfile();
	if (res)
		return res;

	encfp = fopen(ENCRYPTED_NAME, "r");
	
	FILE *decrfp = fopen(DECRYPTED_NAME, "w");
	decrypt_(&encfp, &decrfp, KEY, keylen);

	mu_assert(
		"Decryption failed",
		are_files_equal(&decrfp, &plainfp));
	fclose(plainfp);
	fclose(decrfp);
	fclose(encfp);
	
	remove(DECRYPTED_NAME);
	return 0;
}

static char * all_tests() {
	mu_run_test(test_encrypt_decrypt);
	return 0;
}

int main(void) {
	char *result = all_tests();
	if (result != 0)
		printf("%s\n", result);
	else 
		puts("ALL TESTS PASSED!");
	printf("Tests run: %d\n", tests_run);
	return result != 0;
}
