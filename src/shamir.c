#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
//I need include from here
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include "shamir.h"

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
	printf("\n");
	for (i=0; i<nterms; i++) {
		mpz_out_str(stdout, 10, (*poly)[i]);
		puts(",");
	}
	puts("");
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

/**
 * Evaluates 'x' on the polynomial 'poly'. The result is a real number.
 * @param x: integer value.
 * @param poly: polynomial to be evaluated.
 * @param nterms: size of polynomial.
 * @param res: mpz number, the result will be stored here.
 */
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
 * Frees an array of struct SHARES.
 * æparam shares: array of struct SHARES.
 * @param n: size of the array.
 */
void free_shares(struct SHARE_** shares, int n) {
	int i;
	for (i=0; i<n; i++) {
		mpz_clear((*shares)[i].x);
		mpz_clear((*shares)[i].y);
	}
	free(*shares);
}

/**
 * Prints an array of SHARES.
 */
void print_shares(struct SHARE_** shares, int n, FILE* file) {
	int i;
	for (i=0; i<n; i++) {
		fputs("(", file);
		mpz_out_str(file, 10, (*shares)[i].x);
		fputs(", ", file);
		mpz_out_str(file, 10, (*shares)[i].y);
		fputs(")\n", file);
	}
	fclose(file);
}

void create_shares(int nshares, int min, mpz_t secret, FILE* file) {
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
	print_shares(&shares, nshares, file);

	clear_polynomial(&polynomial, nterms);
	free_shares(&shares, nshares);
}

/**
 * Given the file, it calculates the number of evaluations.
 * @param read: pointer to the file given.
 */
int define_length(FILE *read){
	int read_int;
	char *l = malloc(201*sizeof(char));
	int i=0;
	size_t *n;
	while ((read_int = getline(&l, n, read)) != -1)
		i++;
	free(l);
	return i;
}

/**
 * Given the file, it reads each one of the evaluations, and splits them into
 * the x and y component. Saving them in a structure.
 * @param read: pointer to the file given.
 * @param evaluations: an array of struct's SHARE_ 
 */
void reader(FILE *read, struct SHARE_ **evals){
	int i = 0;
	char* l1 = malloc(201 * sizeof(char));
	size_t n=201;
	rewind(read);
	int read_int;
	while ((read_int = getline(&l1, &n, read)) != -1) {
		mpz_init2((*evals)[i].x, MPZ_LIMIT);
		mpz_init2((*evals)[i].y, MPZ_LIMIT);
		char n1[100];
		char n2[100];
		sscanf(l1, "%s %s", n1, n2);
		mpz_set_str((*evals)[i].x, n1, 10);
		mpz_set_str((*evals)[i].y, n2, 10);
		i++;
	}
	free(l1);
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
		mpz_mul_si((*aux)[i][0], (*aux)[i][0], -1);
		mpz_set_ui((*aux)[i][1], 1);
	}
}

void init_polynomial(mpz_t** poly, int size) {
	int i;
	for (i=0; i < size; i++)
		mpz_init2((*poly)[i], MPZ_LIMIT);
}

void copy_polynomial(mpz_t **a, mpz_t **b, int size) {
	int i;
	for (i=0; i < size; i++)
		mpz_set((*a)[i], (*b)[i]);
}

void mult_polynomials(
		mpz_t **poly, mpz_t *a, mpz_t *b, int al, int bl){

	int i, j;
	mpz_t aux;
	mpz_init2(aux, MPZ_LIMIT);
	mpz_t *ac = malloc((al + 1) * sizeof(mpz_t));
	init_polynomial(&ac, al+1);
	copy_polynomial(&ac, poly, al + 1);	

	init_polynomial(poly, al+1);
	for (i = 0; i <= al; i++){
		for (j = 0; j <= bl; j++){
			mpz_mul(aux, (ac)[i], (b)[j]);
			mpz_add((*poly)[i+j], (*poly)[i+j], aux);
		}
	}
}

/**
 * Frees a mxn matrix of mpz_t
 */
void free_matrix(mpz_t*** matrix, int m, int n) {
	int i;
	for (i=0; i<m; i++) {
		clear_polynomial(&((*matrix)[i]), n);
	}
	free(*matrix);
}

void fill_aux_polynomial(mpz_t*** aux, int length) {
	int i, j;
	for(i = 0; i < length; i++){
		(*aux)[i] = malloc((length) * sizeof(mpz_t));
		for(j = 0; j < length; j++){
			mpz_init2((*aux)[i][j], MPZ_LIMIT);
		}
	}
}

void create_x0_minus_x1_polynomial(mpz_t** poly, mpz_t x0, mpz_t x1) {
	mpz_t minus_x1;
	mpz_init2(minus_x1, MPZ_LIMIT);
	mpz_mul_si(minus_x1, x1, -1);

	//x - x1
	mpz_set((*poly)[0], minus_x1);
	mpz_set((*poly)[1], x0);
}


void calculate_denominator(
		struct SHARE_ **evals, int i, int auxlen, mpz_t *res) {

	mpz_t dvalue;
	mpz_t aux;
	mpz_t accum;

	mpz_init2(aux, MPZ_LIMIT);
	mpz_init2(dvalue, MPZ_LIMIT);
	mpz_init2(accum, MPZ_LIMIT);

	mpz_set(dvalue, (*evals)[i].x);

	int index = 0;
	if (i == 0)
		index = 1;
	mpz_sub(accum, dvalue, (*evals)[index].x);
	int j;
	for (j = index + 1; j<auxlen; j++) {
		if (j == i)
			continue;
		mpz_sub(aux, dvalue, (*evals)[j].x);
		mpz_mul(accum, accum, aux);
	}
	mpz_set(*res, accum);
}

void create_basis_polynomial(
		mpz_t*** aux, struct SHARE_ **evals, int i, int auxlen) {

	int j;
	mpz_t* temp;
	int index = 0;
	if (i == 0)
		index = 1;

	mpz_t one;
	mpz_t den;
	mpz_init2(one, MPZ_LIMIT);
	mpz_init2(den, MPZ_LIMIT);

	mpz_set_ui(one, 1);
	create_x0_minus_x1_polynomial(&((*aux)[i]), one, (*evals)[index].x);
	int grade_accumulated = 1;
	for (j = index + 1; j < auxlen; j++) {
		if (i==j)
			continue;
		temp = malloc(2 * sizeof(mpz_t));
		init_polynomial(&temp, 2);
		create_x0_minus_x1_polynomial(&temp, one, (*evals)[j].x);

		mult_polynomials(
				&((*aux)[i]), (*aux)[i], temp, grade_accumulated, 1);
		grade_accumulated++;
		clear_polynomial(&temp, 2);
	}
	calculate_denominator(evals, i, auxlen, &den);
	/*mpz_out_str(stdout, 10, den);
	  puts("");*/

	for (j=0; j < auxlen; j++) 
		mpz_cdiv_q((*aux)[i][j], ((*aux)[i][j]), den);
}

void multiply_polynomial_by_mpz(mpz_t **poly, mpz_t n, int length) {
	int i;
	for (i=0; i<length; i++)
		mpz_mul((*poly)[i], (*poly)[i], n);
}

/**
 * Given all the evaluations, it generates the basis polynomials for the 
 * lagrange reconstruction.
 * @param evaluations: an array of struct's SHARE_ 
 * @param poly: the array where the polynomial basis will be save
 * @param length: the number of evaluations
 */
void lagrange_basis(
		struct SHARE_ **evals, mpz_t ***aux, int length){

	int i;
	for (i=0; i < length; i++) {
		create_basis_polynomial(aux, evals, i, length);
		//print_polynomial((&((*aux)[i])), length);
	}	
	//print polynomial
	//free_matrix(aux, length, length - 1);
}

void sum_polynomials(mpz_t **poly, mpz_t** a, mpz_t** b, int size) {
	int i;
	for (i=0; i<size; i++)
		mpz_add((*poly)[i], (*a)[i], (*b)[i]);
}

void rebuild_polynomial(
		mpz_t **polynomial, mpz_t ***aux,
		struct SHARE_ **evals, int length) {

	int i;
	for (i=0; i<length; i++) {
		multiply_polynomial_by_mpz(&((*aux)[i]), (*evals)[i].y, length);
	}
	copy_polynomial(polynomial, &((*aux)[0]), length);
	for (i=1; i<length; i++)
		sum_polynomials(polynomial, polynomial, &((*aux)[i]), length);
}


/*int main(int argc, char *args[]) {

  FILE *read = fopen(args[1], "r");
  int length = define_length(read);
//if length = 1, i.e, there is only 1 point, an error ocurrs
struct SHARE_* evals = malloc(sizeof(struct SHARE_) * length-1);
reader(read, &evals);
int i;
int j;
mpz_t * poly = malloc(length * sizeof(mpz_t));
init_polynomial(&poly, length);	

mpz_t **aux = malloc(length * sizeof(mpz_t*));
fill_aux_polynomial(&aux, length);

lagrange_basis(&evals, &aux, length);
rebuild_polynomial(&poly, &aux, &evals, length);
/*mpz_t * reconstructed_poly = malloc(length * sizeof(mpz_t));
for (i = 0; i < length; i++){
mpz_init2(reconstructed_poly[i], MPZ_LIMIT);
}
lagrange_basis(&evaluations, &poly, length);
lagrange_reconstruction(&evaluations, &poly, length, &reconstructed_poly);
*/

/*puts("polynomial:");
  print_polynomial(&poly, length);

/*mpz_t secret;
mpz_init(secret);
mpz_set_ui(secret, 2);*/

/*int nshares = 5;
  int min = 4;
  */
//mpz_t* polynomial = malloc((min - 1) * sizeof(mpz_t));

//build_polynomial(&polynomial, min - 1, secret);
//print_polynomial(&polynomial, min - 1);
// mpz_out_str(stdout, 10, secret);
//create_shares(nshares, min, secret);
//clear_polynomial(&polynomial, min - 1);
/*clear_polynomial(&poly, length);
  free_matrix(&aux, length, length);
  fclose(read);
// mpz_clear(secret);
return 0;
}*/
