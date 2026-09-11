#include <gmp.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void get_random_n_bits(mpz_t r, int bits, gmp_randstate_t state) {
  mpz_urandomb(r, state, bits);
}

void get_random_prime(mpz_t p, int bits, gmp_randstate_t state) {
  mpz_urandomb(p, state, bits);
  mpz_nextprime(p, p);
}

// Paillier structure
typedef struct {
  mpz_t n;
  mpz_t n_sq;
  mpz_t g;
  mpz_t lambda;
  mpz_t mu;
} PaillierContext;

void paillier_init(PaillierContext *ctx, int key_bits, gmp_randstate_t state) {
  mpz_t p, q, pm1, qm1, gcd_val;
  mpz_inits(p, q, pm1, qm1, gcd_val, NULL);
  mpz_inits(ctx->n, ctx->n_sq, ctx->g, ctx->lambda, ctx->mu, NULL);

  // Generate p, q
  get_random_prime(p, key_bits / 2, state);
  get_random_prime(q, key_bits / 2, state);

  // n = p*q
  mpz_mul(ctx->n, p, q);

  // n^2
  mpz_mul(ctx->n_sq, ctx->n, ctx->n);

  // g = n + 1
  mpz_add_ui(ctx->g, ctx->n, 1);

  // lambda = lcm(p-1, q-1)
  mpz_sub_ui(pm1, p, 1);
  mpz_sub_ui(qm1, q, 1);
  mpz_lcm(ctx->lambda, pm1, qm1);

  // mu = lambda^-1 mod n
  mpz_invert(ctx->mu, ctx->lambda, ctx->n);

  mpz_clears(p, q, pm1, qm1, gcd_val, NULL);
}

void paillier_encrypt(mpz_t c, mpz_t m, PaillierContext *ctx,
                      gmp_randstate_t state) {
  mpz_t r, rn, gm;
  mpz_inits(r, rn, gm, NULL);

  // r random in [1, n-1]
  mpz_urandomm(r, state, ctx->n);

  // rn = r^n mod n^2
  mpz_powm(rn, r, ctx->n, ctx->n_sq);

  // gm = g^m mod n^2 = (1 + m*n) mod n^2  (optimization)
  // gm = 1 + m*n
  mpz_mul(gm, m, ctx->n);
  mpz_add_ui(gm, gm, 1);
  mpz_mod(gm, gm, ctx->n_sq);

  // c = gm * rn mod n^2
  mpz_mul(c, gm, rn);
  mpz_mod(c, c, ctx->n_sq);

  mpz_clears(r, rn, gm, NULL);
}

void paillier_decrypt(mpz_t m, mpz_t c, PaillierContext *ctx) {
  mpz_t u;
  mpz_init(u);

  // u = c^lambda mod n^2
  mpz_powm(u, c, ctx->lambda, ctx->n_sq);

  // L(u) = (u-1)/n
  mpz_sub_ui(u, u, 1);
  mpz_divexact(u, u, ctx->n);

  // m = L(u) * mu mod n
  mpz_mul(m, u, ctx->mu);
  mpz_mod(m, m, ctx->n);

  mpz_clear(u);
}

int main(int argc, char **argv) {
  int count = 100;
  if (argc > 1)
    count = atoi(argv[1]);

  

  // Global state for keygen
  gmp_randstate_t state;
  gmp_randinit_default(state);
  gmp_randseed_ui(state, time(NULL));

  PaillierContext ctx;
  printf("Initializing Paillier CPU (C++/GMP + OpenMP) with 2048-bit "
         "modulus...\n");
  paillier_init(&ctx, 2048, state);

  printf("Running benchmark with %d iterations on %d threads...\n", count,
         omp_get_max_threads());

  mpz_t *msgs = (mpz_t *)malloc(count * sizeof(mpz_t));
  mpz_t *ciphers = (mpz_t *)malloc(count * sizeof(mpz_t));
  mpz_t *decrypted = (mpz_t *)malloc(count * sizeof(mpz_t));

  // Initialize vars
  for (int i = 0; i < count; i++) {
    mpz_inits(msgs[i], ciphers[i], decrypted[i], NULL);
    mpz_urandomm(msgs[i], state, ctx.n); // msg < n
  }

  // Encryption
  double start = omp_get_wtime();

#pragma omp parallel
  {
    gmp_randstate_t thread_state;
    gmp_randinit_default(thread_state);
    gmp_randseed_ui(thread_state, time(NULL) ^ omp_get_thread_num());

#pragma omp for
    for (int i = 0; i < count; i++) {
      paillier_encrypt(ciphers[i], msgs[i], &ctx, thread_state);
    }

    gmp_randclear(thread_state);
  }
  double end_enc = omp_get_wtime();

  // Decryption
  double start_dec = omp_get_wtime();

#pragma omp parallel for
  for (int i = 0; i < count; i++) {
    paillier_decrypt(decrypted[i], ciphers[i], &ctx);
  }
  double end_dec = omp_get_wtime();

  // Verify
  int errors = 0;
  for (int i = 0; i < count; i++) {
    if (mpz_cmp(msgs[i], decrypted[i]) != 0)
      errors++;
  }

  double enc_time = end_enc - start;
  double dec_time = end_dec - start_dec;

  printf("Success! Errors: %d\n", errors);
  printf("Total Encryption Time: %.4f s\n", enc_time);
  printf("Avg Encryption Time: %.4f ms\n", (enc_time * 1000) / count);
  printf("Total Decryption Time: %.4f s\n", dec_time);
  printf("Avg Decryption Time: %.4f ms\n", (dec_time * 1000) / count);

  return 0;
}
```
