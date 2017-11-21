#define MODE "cfb"
#define ALGORITHM "rijndael-256"

int encrypt(
	FILE **plainfp, FILE **encrfp, char* key, int keysize, char *IV);
int decrypt(
	FILE **encrfp, FILE **decrfp, char *key, int keysize, char *IV);
