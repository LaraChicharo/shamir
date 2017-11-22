#include <stdio.h>
#include <mcrypt.h>
#include "cipher.h"


int encrypt(
	FILE **plainfp, FILE **encrfp, char* key, int keysize, char* IV) {
	
	MCRYPT td = mcrypt_module_open(
		ALGORITHM, NULL, MODE, NULL);
	// size in bytes
	int max_key_size = mcrypt_enc_get_key_size(td);
	int iv_size = mcrypt_enc_get_iv_size(td);  // size in bytes

	mcrypt_generic_init(td, key, keysize, IV);
	char block_buffer;
	while (fread(&block_buffer, 1, 1, *plainfp) == 1) {
		mcrypt_generic(td, &block_buffer, 1);
		fwrite(&block_buffer, 1, 1, *encrfp);
	}
	/* Deinit the encryption thread, and unload the module */
	mcrypt_generic_end(td);
	
	return 0;
}

int decrypt(
	FILE **encrfp, FILE **decrfp, char *key, int keysize, char *IV) {
	
	MCRYPT td = mcrypt_module_open(
		ALGORITHM, NULL, MODE, NULL);
	mcrypt_generic_init(td, key, keysize, IV);
	char block_buffer;
	while (fread(&block_buffer, 1, 1, *encrfp) == 1) {
		mdecrypt_generic(td, &block_buffer, 1);
		fwrite(&block_buffer, 1, 1, *decrfp);
	}
	/* Deinit the encryption thread, and unload the module*/
	mcrypt_generic_end(td);
	
	return 0;
}

/*int main(void) {
	
	FILE *plainfp = fopen("zimmermann.txt", "r");
	FILE *encrfp = fopen("encrypted.txt", "w");
	
	char *key = "0123456789abcdef";
	int keysize = 16;
	char *IV = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";  // 32 bytes

	if (encrypt(&plainfp, &encrfp, key, keysize, IV) == 0) {
		fclose(encrfp);
		encrfp = fopen("encrypted.txt", "r");
		printf("%s", "Encryption successful\n");
	}
	
	FILE *decrfp = fopen("decrypted.txt", "w");
	if (decrypt(&encrfp, &decrfp, key, keysize, IV) == 0) {
		fclose(plainfp);
		fclose(encrfp);
		fclose(decrfp);
		printf("%s", "Decryption successful\n");
	} else {
		// Error
	}

	return 0;
}*/
