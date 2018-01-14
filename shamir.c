#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
//I need include from here
#include <x86_64-linux-gnu/gmp.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>

#define URANDOM "/dev/urandom"
#define MPZ_LIMIT 256

struct SHARE_ {
	mpz_t x;
	mpz_t y;
};


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
		puts(",");
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

void eval_polynomial(
	unsigned int x, mpz_t** poly, int nterms, mpz_t* res) {
	mpz_t term_res;
	mpz_init(term_res);

	int i;			
	for (i=0; i<nterms; i++) {
		mpz_mul_ui(term_res, (*poly)[i], pow(x, i));
		mpz_add(*res, term_res, *res);
	}
	mpz_clear(term_res);
}

/**
 * Prints an array of SHARES.
 */
void print_shares(struct SHARE_** shares, int n) {
	int i;
	for (i=0; i<n; i++) {
		printf("%s", "(");
		mpz_out_str(stdout, 10, (*shares)[i].x);
		printf("%s", ", ");
		mpz_out_str(stdout, 10, (*shares)[i].y);
		printf("%s\n", ")");
	}
}

void free_shares(struct SHARE_** shares, int n) {
	int i;
	for (i=0; i<n; i++) {
		mpz_clear((*shares)[i].x);
		mpz_clear((*shares)[i].y);
	}
	free(*shares);
}

void create_shares(int nshares, int min, mpz_t secret) {
	if (nshares < min) {
		fprintf(
			stderr,
			"%s\n",
			"The number of shares cant be less than min.");
		exit(EINVAL);  // Invalid argument
	}
	int nterms = min - 1;
	mpz_t* polynomial = malloc(nterms * sizeof(mpz_t));
	build_polynomial(&polynomial, nterms, secret);
	print_polynomial(&polynomial, nterms);
	struct SHARE_* shares = malloc(sizeof(struct SHARE_) * nshares);
		
	unsigned int i;
	for (i=0; i<nshares; i++) {
		mpz_init2(shares[i].x, MPZ_LIMIT);
		mpz_init2(shares[i].y, MPZ_LIMIT);
		mpz_set_ui(shares[i].x, i);
		eval_polynomial(i, &polynomial, nterms, &shares[i].y);
	}
	print_shares(&shares, nshares);

	clear_polynomial(&polynomial, nterms);
	free_shares(&shares, nshares);
}

int define_length(read);
void reader(FILE* read, struct SHARE_ **evaluations);
void lagrange_reconstruction(struct SHARE_ **evaluations);

int main(int argc, char *args[]) {

	FILE *read = fopen(args[1], "r");
	int length = define_length(read);
	struct SHARE_* evaluations = malloc(sizeof(struct SHARE_) * length-1);
	reader(read, &evaluations);
	lagrange_reconstruction(&evaluations);

	mpz_t secret;
	mpz_init(secret);
	mpz_set_ui(secret, 2);

	int nshares = 5;
	int min = 4;
	//mpz_t* polynomial = malloc((min - 1) * sizeof(mpz_t));
		
	//build_polynomial(&polynomial, min - 1, secret);
	//print_polynomial(&polynomial, min - 1);
	// mpz_out_str(stdout, 10, secret);
	create_shares(nshares, min, secret);
	//clear_polynomial(&polynomial, min - 1);

	mpz_clear(secret);
	return 0;
}

int define_length(FILE *read){
	int length = 0;
	char block[255];
	while(!feof(read)){
		fscanf(read, "%s", block);
		fscanf(read, "%s", block);
		length++;
	}
	return length;
}

void reader(FILE *read, struct SHARE_ **evaluations){
	char block[1024];
	int i = 0;
	rewind(read);
	while(!feof(read)){
		mpz_init2((*evaluations)[i].x, MPZ_LIMIT);
		mpz_init2((*evaluations)[i].y, MPZ_LIMIT);
		fscanf(read, "%s", block);
		mpz_set_str((*evaluations)[i].x, block, 3);
		fscanf(read, "%s", block);
		mpz_set_str((*evaluations)[i].y, block, 3);
		/*printf("%s", "(");
		mpz_out_str(stdout, 10, *evaluations.x);
		printf("%s", ", ");
		mpz_out_str(stdout, 10, *evaluations.y);
		printf("%s\n", ")");*/
		i++;
	}
}

void lagrange_reconstruction(struct SHARE_ **evaluations){

}
