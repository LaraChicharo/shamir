#include <stdio.h>
#include <mhash.h>
#include <string.h>
#include <math.h>


/**
 *Takes the byte representation of a string and converts it to int.
 *@param str: pointer to string.
 *@param str_size: size of the string (including null byte).
 *@param buff: store string binary representation here.
 *It must be (((str_size - 1) * 8) + 1) long.
 *@returns number.
 */
void string_as_binary(
	char **str, unsigned int str_size, char **buff) {
	
	int i;
	int j;
	char k;
	for (i = 0; i < str_size - 1; i++) {
		k = (*str)[i];
		for (j = 7; j >= 0 ; j--) {
			if ((k >> j) & 1)
				(*buff)[i*8 + 7-j] = '1';
			else
				(*buff)[i*8 + 7-j] = '0';
		}
	}
	puts("done with for");
	(*buff)[str_size*8 + 1] = '\0';
}

void hash_string(char **str, char **buff) {
	MHASH mh = mhash_init(MHASH_SHA256);
	mhash(mh, *str, strlen(*str));
	mhash_deinit(mh, *buff);
}

void hash_string_to_int(char **str, char **buff) {
	int str_len = strlen(*str);
	char *tempbuff = malloc((sizeof(char) * 32) + 1);
	hash_string(str, &tempbuff);
	string_as_binary(&tempbuff, 33, buff);
	free(tempbuff);
}

int main(int argc, char** argv) {
	int str_len = strlen(argv[1]);
	char *buff = malloc((sizeof(char) * 32 * 8) + 1);
	hash_string_to_int(&argv[1], &buff);
	/*string_as_binary(
		&argv[1], str_len + 1, &buff);*/
	printf("binary repr: %s\n", buff);
	free(buff);
	return 0;
}

