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

/**
 * A structure that retains two values. Designed to be used as a
 * coordinate point
 */
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

/**
 * Given the file, it calculates the number of evaluations.
 * @param read: pointer to the file given.
 */
int define_length(FILE *read){
	int length = 0;
	char block[255];
	while(!feof(read)){
		fscanf(read, "%s", block);
		fscanf(read, "%s", block);
		length++;
	}
	return length-1;
}

/**
 * Given the file, it reads each one of the evaluations, and splits them into
 * the x and y component. Saving them in a structure.
 * @param read: pointer to the file given.
 * @param evaluations: an array of struct's SHARE_ 
 */
void reader(FILE *read, struct SHARE_ **evaluations){
	char block[1024];
	int i = 0;
	rewind(read);
	while(!feof(read)){
		mpz_init2((*evaluations)[i].x, MPZ_LIMIT);
		mpz_init2((*evaluations)[i].y, MPZ_LIMIT);
		fscanf(read, "%s", block);
		mpz_set_str((*evaluations)[i].x, block, 10);
		fscanf(read, "%s", block);
		mpz_set_str((*evaluations)[i].y, block, 10);
		i++;
	}
}

/**
 * It creates the polynomials respective to the evaluation points for the 
 * lagrange_basis usage
 */
void create_eval_polynomials(struct SHARE_ **evaluations, mpz_t ***aux, int length){
	/*the independet value of the polynom will be the x-component of 
	  the point given*/
	int i;
	for(i = 0; i < length; i++){
		mpz_set((*aux)[i][0], (*evaluations)[i].x);
		mpz_set_ui((*aux)[i][1], 1);
	}
}



/**
 * It reallocates the positions of the array in order to do the correct 
 * polynomial products for the lagrange_basis
 */
void right_position(mpz_t ***aux, int length){
	int i,j;
	mpz_t temp[2];
	mpz_init2(temp[0], MPZ_LIMIT);
	mpz_init2(temp[1], MPZ_LIMIT);
	mpz_set(temp[0], (*aux)[0][0]);
	mpz_set(temp[1], (*aux)[0][1]);
	for(i = 0; i < length-1; i++){
		for(j = 0; j < 2; j++){
			mpz_set((*aux)[i][j], (*aux)[i+1][j]);
		}
	}
	mpz_set((*aux)[length-1][0], temp[0]);
	mpz_set((*aux)[length-1][1], temp[1]);
}

/**
 * It makes the product of two polynomails and then save the result in a
 * matrix (poly) in the correspondent position. In order to assist the
 * mult_polyinomails.
 */
void mult_assistant(mpz_t **op1, mpz_t **op2, mpz_t ***poly, int position){
	int size1 = sizeof(*op1)/sizeof((*op1)[0]);
	int size2 = sizeof(*op2)/sizeof((*op2)[0]);
	int i,j;
	for (i = 0; i < size1; i++){
		for (j = 0; j < size2; j++){
			//(*poly)[position][i+j] += (*op1)[i] + (*op2)[j];
		}
	}
}

/**
 * It makes the product of all the polynomials needed for each lagrange 
 * polynomial-basis'. Then it saves the resulting polynomial in a matrix.
 * The position represents wich point of the evaluations won't be used.
 */
void mult_polyinomails(mpz_t ***aux, int length, mpz_t ***poly, int position){
	int i;
	mult_assistant(&((*aux)[0]), &((*aux)[1]), poly, position);
	if(length != 3){
		for(i = 2; i < length; i++){
			mult_assistant(&((*aux)[2]), &((*poly)[position]), poly, position);
		}
	}
}

/**
 * Given all the evaluations, it generates the basis polynomials for the 
 * lagrange reconstruction.
 * @param evaluations: an array of struct's SHARE_ 
 * @param poly: the array where the polynomial basis will be save
 * @param length: the number of evaluations
 */
void lagrange_basis(
	struct SHARE_ **evaluations, mpz_t ***poly, int length){
	int i, j;

	//case if the length is 2

	/*for(i = 0; i < length; i++){
		for(j = 0; j < length; j++){
			mpz_set_ui((*poly)[i][j], 0);
		}
	}*/

	//where basis-polynomials will be allocated
	//mpz_t **aux[length][2];
	mpz_t **aux = malloc(length * sizeof(mpz_t*));
	for(i = 0; i < length; i++){
		aux[i] = malloc(2 * sizeof(mpz_t));
		for(j = 0; j < 2; j++){
			mpz_init2(aux[i][j], MPZ_LIMIT);
		}
	}

	create_eval_polynomials(evaluations, &aux, length);
	
	for(i = 0; i < length; i++){
		right_position(&aux, length);
		mult_polyinomails(&aux, length, poly, i);
	}
}

int main(int argc, char *args[]) {

	FILE *read = fopen(args[1], "r");
	int length = define_length(read);
	//if length = 1, i.e, there is only 1 point, an error ocurrs
	struct SHARE_* evaluations = malloc(sizeof(struct SHARE_) * length);
	reader(read, &evaluations);
	// mpz_t poly[length][length];
	mpz_t ** poly = malloc(length * sizeof(mpz_t*));
	int i;
	for (i=0; i<length; i++) {
		poly[i] = malloc(length * sizeof(mpz_t));
		int j;
		for (j=0; j<length; j++)
			mpz_init2(poly[i][j], MPZ_LIMIT);
	}
	lagrange_basis(&evaluations, &poly, length);

	mpz_t secret;
	mpz_init(secret);
	mpz_set_ui(secret, 2);

	int nshares = 5;
	int min = 4;
	//mpz_t* polynomial = malloc((min - 1) * sizeof(mpz_t));
		
	//build_polynomial(&polynomial, min - 1, secret);
	//print_polynomial(&polynomial, min - 1);
	// mpz_out_str(stdout, 10, secret);
	//create_shares(nshares, min, secret);
	//clear_polynomial(&polynomial, min - 1);

	mpz_clear(secret);
	return 0;
}
