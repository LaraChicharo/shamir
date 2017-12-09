#include <stdio.h>
#include <mhash.h>
#include <string.h>


int hash_string(char **str, char **buff) {
	MHASH mh = mhash_init(MHASH_SHA256);
	mhash(mh, *str, strlen(*str));
	mhash_deinit(mh, *buff);
}

int main(int argc, char** argv) {
	char *hash = malloc(sizeof(char) * 33);
	hash_string(&argv[1], &hash);
	printf(
		"pass: %s, hash:%s, hashlen:%d\n",
		argv[1], hash, strlen(hash));
	free(hash);
	return 0;
}
