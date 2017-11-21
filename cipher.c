#include <stdio.h>
#include <mcrypt.h>


int encrypt(FILE **plainfp, FILE **encrfp) {
	MCRYPT td = mcrypt_module_open(
		"rijndael-256", NULL, "cfb", NULL);
	int max_key_size = mcrypt_enc_get_key_size(td);  // size in bytes
	int iv_size = mcrypt_enc_get_iv_size(td);  // size in bytes
	char block_buffer;
	
	char *key = "0123456789abcdef";
	int keysize = 16;
	char *IV = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";  // 32 bytes
	
	
	mcrypt_generic_init(td, key, keysize, IV);
	
	while (fread(&block_buffer, 1, 1, *plainfp)) {
		mcrypt_generic(td, &block_buffer, 1);
		fwrite(&block_buffer, 1, 1, *encrfp);
	}
	/* Deinit the encryption thread, and unload the module */
	mcrypt_generic_end(td);
	return 0;
}

int main(void) {
	
	FILE *plainfp = fopen("zimmermann.txt", "r");
	FILE *encrfp = fopen("encrypted.txt", "w");
	
	if (encrypt(&plainfp, &encrfp) == 0) {
		fclose(plainfp);
		fclose(encrfp);
	} else
		// Error
	return 0;
}
