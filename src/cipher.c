#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mcrypt.h>
#include "cipher.h"


/**
 * Encrypts a file.
 * @param plainfp pointer to file to be encrypted.
 * @param encrfp ptr to file where the encrypted data will be stored.
 * @param key password to be used encrypt.
 * @param keysize size of key.
 * @returns status code. 0 for OK.
 */
int encrypt_(
	FILE **plainfp, FILE **encrfp, char* key, int keysize) {
	
	MCRYPT td = mcrypt_module_open(
		ALGORITHM, NULL, MODE, NULL);
	// size in bytes
	int max_key_size = mcrypt_enc_get_key_size(td);
	char *real_key = malloc(max_key_size * sizeof(char));
	
	puts("before strncpy");		
	strncpy(real_key, key, max_key_size - 1);
	puts("after strncpy");
	real_key[max_key_size - 2] = '\0'; 
	puts("afeter null char");

	mcrypt_generic_init(td, real_key, max_key_size - 1, IV);
	char block_buffer;
	while (fread(&block_buffer, 1, 1, *plainfp) == 1) {
		mcrypt_generic(td, &block_buffer, 1);
		fwrite(&block_buffer, 1, 1, *encrfp);
	}
	/* Deinit the encryption thread, and unload the module */
	mcrypt_generic_end(td);
	free(real_key);
	
	return 0;
}

/**
 * Decrypts a file.
 * @param encrfp ptr to file where the encrypted data is be stored.
 * @param decrfp ptr to file where the decrypted data will be saved.
 * @param key password to be used to decrypt.
 * @param keysize size of key.
 */
int decrypt_(
	FILE **encrfp, FILE **decrfp, char* key, int keysize) {
	
	MCRYPT td = mcrypt_module_open(
		ALGORITHM, NULL, MODE, NULL);
	// size in bytes
	int max_key_size = mcrypt_enc_get_key_size(td);
	
	char *real_key = malloc(max_key_size * sizeof(char));
	strncpy(real_key, key, max_key_size - 1);
	real_key[max_key_size - 2] = '\0'; 
	
	mcrypt_generic_init(td, real_key, max_key_size - 1, IV);
	char block_buffer;
	while (fread(&block_buffer, 1, 1, *encrfp) == 1) {
		mdecrypt_generic(td, &block_buffer, 1);
		fwrite(&block_buffer, 1, 1, *decrfp);
	}
	/* Deinit the encryption thread, and unload the module*/
	mcrypt_generic_end(td);
	free(real_key);	
	return 0;
}

