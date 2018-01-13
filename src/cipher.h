#define MODE "cfb"
#define ALGORITHM "rijndael-256"
// static IV for now.
#define IV "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"  // 32 bytes
// size varies from one algorithm to another, other factors may affect.
// use mcrypt_enc_get_iv_size(MCRYPT td) to find out the necessary size

int encrypt(
	FILE **plainfp, FILE **encrfp, char* key, int keysize);
int decrypt(
	FILE **encrfp, FILE **decrfp, char *key, int keysize);
