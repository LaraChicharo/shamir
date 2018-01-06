#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <gmp.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>

#define URANDOM "/dev/urandom"
#define MPZ_LIMIT 256

/*struct SHARE {
	struct mpz_t *x;
	struct mpz_t *y;
};
*/

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
		char rd[8];  // to make a 64 bit int
		if (read(rdata, rd, sizeof rd) < 0) {
			close(rdata);
			error_handling_urandom();
		}
		close(rdata);
		return bytes_to_ulong(rd);
	}
}

/**
 * Given the mpz polynomial 'poly', fills it with random terms,
 * Except the independent term, it associates to it a given number.
 * @param poly: polynomial to fill.
 * @param state: Initialized state to generate random ints with mpz.
 * @param nterms: size of the polynomial.
 */
void fill_polynomial(
	mpz_t** poly, gmp_randstate_t state, int nterms,
	int limit, mpz_t secret) {
	mpz_init2((*poly)[0], limit);
	mpz_set((*poly)[0], secret);
	int i;
	for (i=1; i < nterms; i++) {
		mpz_init2((*poly)[i], limit);
		mpz_urandomb((*poly)[i], state, limit);
	}
}

/**
 * Frees a given polynomial from memory.
 * @param poly: the polynomial to fill,
 * @param nterms: size of the polynomial.
 */
void clear_polynomial(mpz_t** poly, int nterms) {
	int i;
	for (i = 0; i < nterms; i++) {
		mpz_clear((*poly)[i]);
	}
	free(*poly);
}

/**
 * Prints a given polynomial.
 * @param poly: the polynomial to print,
 * @param nterms: size of the polynomial.
 */
void print_polynomial(mpz_t** poly, int nterms) {
	int i;
	for (i=0; i<nterms; i++) {
		mpz_out_str(stdout, 10, (*poly)[i]);
		puts("\n");
	}
}

/**
 * Given a mpz polynomial, fills it with random terms,
 * Except the independent term, it associates to it a given number.
 * @param polynomial: polynomial to fill.
 * @param nterms: size of the polynomial.
 * @param secret: number that will be placed as the independent term.
 */
void build_polynomial(mpz_t** polynomial, int nterms, mpz_t secret) {
	
	gmp_randstate_t state;
	gmp_randinit_default(state);
	gmp_randseed_ui(state, read_ulong_urandom());
	fill_polynomial(polynomial, state, nterms, MPZ_LIMIT, secret);
	gmp_randclear(state);
}


int main(void) {

	mpz_t secret;
	mpz_init(secret);
	mpz_set_ui(secret, 2);

	int nterms = 5;
	mpz_t* polynomial = malloc(nterms * sizeof(mpz_t));
	build_polynomial(&polynomial, nterms, secret);

	print_polynomial(&polynomial, nterms);
	clear_polynomial(&polynomial, nterms);
	mpz_clear(secret);
	return 0;
}
