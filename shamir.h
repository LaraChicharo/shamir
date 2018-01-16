#define URANDOM "/dev/urandom"
#define MPZ_LIMIT 256
//#define _GNU_SOURCE

/**
 * A structure that retains two values. Designed to be used as a
 * coordinate point
 */
struct SHARE_ {
	mpz_t x;
	mpz_t y;
};

unsigned long long bytes_to_ulong(char bytes[8]);

void error_handling_urandom(void);

unsigned long long read_ulong_urandom(void);

void fill_polynomial(
	mpz_t** poly, gmp_randstate_t state, int nterms,
	int limit, mpz_t secret);

void clear_polynomial(mpz_t** poly, int nterms);

void print_polynomial(mpz_t** poly, int nterms);

void build_polynomial(mpz_t** polynomial, int nterms, mpz_t secret);

void eval_polynomial(
	unsigned int x, mpz_t** poly, int nterms, mpz_t* res);

void print_shares(struct SHARE_** shares, int n, FILE* file);

void free_shares(struct SHARE_** shares, int n);

void create_shares(int nshares, int min, mpz_t secret, FILE* file);

int define_length(FILE *read);

void reader(FILE *read, struct SHARE_ **evals);

void create_eval_polynomials(struct SHARE_ **evaluations, mpz_t ***aux, int length);

void init_polynomial(mpz_t** poly, int size);

void copy_polynomial(mpz_t **a, mpz_t **b, int size);

void mult_polynomials(
	mpz_t **poly, mpz_t *a, mpz_t *b, int al, int bl);

void free_matrix(mpz_t*** matrix, int m, int n);

void fill_aux_polynomial(mpz_t*** aux, int length);

void create_x0_minus_x1_polynomial(mpz_t** poly, mpz_t x0, mpz_t x1);

void calculate_denominator(
	struct SHARE_ **evals, int i, int auxlen, mpz_t *res);

void create_basis_polynomial(
	mpz_t*** aux, struct SHARE_ **evals, int i, int auxlen);

void multiply_polynomial_by_mpz(mpz_t **poly, mpz_t n, int length);

void lagrange_basis(
	struct SHARE_ **evals, mpz_t ***aux, int length);

void sum_polynomials(mpz_t **poly, mpz_t** a, mpz_t** b, int size);

void rebuild_polynomial(
	mpz_t **polynomial, mpz_t ***aux,
	struct SHARE_ **evals, int length);