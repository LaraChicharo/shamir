#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <gmp.h>
#include <fcntl.h>
#include <errno.h>

#define URANDOM "/dev/urandom"

/**
 * Converts 8 bytes into an unsigned long long
 * More bytes wouldn't be ensured to fit into a long long.
 * Using little endian scheme.
 * @param bytes: the bytes to convert.
 * @returns the number in its integer form.
 */
unsigned long long bytes_to_ulong(char bytes[8]) {
	unsigned long long n = 0;
	int i;
	for (i=0; i<sizeof bytes; i++)
		n += bytes[i] << i*8;
	return n;
	
}

/**
 * Handles errors when dealing with /dev/urandom
 */
void error_handling_urandom(void) {
	fprintf(
		stderr,
		"Error interacting with%s: \n%s\n",
		URANDOM,
		strerror(errno));
	exit(errno);	
}

/**
 * Reads bytes from urandom to get an unsigned integer.
 * @return randomly generated unsigned integer.
 */
unsigned long long read_ulong_urandom(void) {
	int rdata;
	rdata = open(URANDOM, O_RDONLY);
	if (rdata < 0) {
		close(rdata);
		error_handling_urandom();
	} else {
		char rd[8];  // 64 bit
		if (read(rdata, rd, sizeof rd) < 0) {
			close(rdata);
			error_handling_urandom();
		}
		close(rdata);
		return bytes_to_ulong(rd);
	}
}

/**
 * Example generating a random mpz number.
 */
void print_mpz_randint(void) {
	mpz_t rand;
	mpz_init(rand);
	gmp_randstate_t state;
	gmp_randinit_default(state);
	gmp_randseed_ui(state, read_ulong_urandom());
	mpz_urandomb(rand, state, 8);

	mpz_out_str(stdout, 10, rand);
	printf("%s","\n");
	gmp_randclear(state);
	mpz_clear(rand);
}

int main(void) {
	print_mpz_randint();
	return 0;
}
