#include <gmp.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void get_random_prime(mpz_t p, int bits, gmp_randstate_t state) {
  mpz_urandomb(p, state, bits);
  mpz_nextprime(p, p);
}

// Okamoto-Uchiyama key structure
typedef struct {
  mpz_t n;    // p^2 * q
  mpz_t g;    // in (Z/nZ)*
  mpz_t h;    // g^n mod n
  mpz_t p, q; // Private key
  mpz_t p_sq; // p^2
  mpz_t pm1;  // p-1
} OUContext;

void ou_init(OUContext *ctx, int key_bits, gmp_randstate_t state) {
  mpz_inits(ctx->n, ctx->g, ctx->h, ctx->p, ctx->q, ctx->p_sq, ctx->pm1, NULL);

  // Generate p, q (approx key_bits/3 since n = p^2 q)
  // Actually standard: p, q roughly same size. n len = 3 * prime_len
  get_random_prime(ctx->p, key_bits / 3, state);
  get_random_prime(ctx->q, key_bits / 3, state);

  // n = p^2 * q
  mpz_mul(ctx->p_sq, ctx->p, ctx->p);
  mpz_mul(ctx->n, ctx->p_sq, ctx->q);

  // g in Z/nZ*. Choose random g < n.
  // Condition: g^(p-1) mod p^2 != 1
  mpz_t temp;
  mpz_init(temp);
  mpz_sub_ui(ctx->pm1, ctx->p, 1);

  while (1) {
    mpz_urandomm(ctx->g, state, ctx->n);
    if (mpz_cmp_ui(ctx->g, 2) < 0)
      continue;

    // Check condition
    mpz_powm(temp, ctx->g, ctx->pm1, ctx->p_sq);
    if (mpz_cmp_ui(temp, 1) != 0)
      break;
  }

  // h = g^n mod n
  mpz_powm(ctx->h, ctx->g, ctx->n, ctx->n);

  mpz_clear(temp);
}

void ou_encrypt(mpz_t c, mpz_t m, OUContext *ctx, gmp_randstate_t state) {
  // c = g^m * h^r mod n
  // m < p

  mpz_t r, t1, t2;
  mpz_inits(r, t1, t2, NULL);

  mpz_urandomm(r, state, ctx->n);

  // t1 = g^m mod n
  mpz_powm(t1, ctx->g, m, ctx->n);

  // t2 = h^r mod n
  mpz_powm(t2, ctx->h, r, ctx->n);

  // c = t1 * t2 mod n
  mpz_mul(c, t1, t2);
  mpz_mod(c, c, ctx->n);

  mpz_clears(r, t1, t2, NULL);
}

// L(x) = (x-1)/p
void L_func(mpz_t res, mpz_t x, mpz_t p) {
  mpz_sub_ui(res, x, 1);
  mpz_divexact(res, res, p);
}

void ou_decrypt(mpz_t m, mpz_t c, OUContext *ctx) {
  // m = L(c^(p-1) mod p^2) / L(g^(p-1) mod p^2) mod p
  // Denominator can be precomputed.

  mpz_t cp, gp, l_cp, l_gp, inv_l_gp;
  mpz_inits(cp, gp, l_cp, l_gp, inv_l_gp, NULL);

  // cp = c^(p-1) mod p^2
  mpz_powm(cp, c, ctx->pm1, ctx->p_sq);

  // gp = g^(p-1) mod p^2 (This should ideally be precomputed in context)
  mpz_powm(gp, ctx->g, ctx->pm1, ctx->p_sq);

  L_func(l_cp, cp, ctx->p);
  L_func(l_gp, gp, ctx->p);

  mpz_invert(inv_l_gp, l_gp, ctx->p);

  mpz_mul(m, l_cp, inv_l_gp);
  mpz_mod(m, m, ctx->p);

  mpz_clears(cp, gp, l_cp, l_gp, inv_l_gp, NULL);
}

int main(int argc, char **argv) {
  int count = 100;
  if (argc > 1)
    count = atoi(argv[1]);

  // Global state for keygen
  gmp_randstate_t state;
  gmp_randinit_default(state);
  gmp_randseed_ui(state, time(NULL));

  OUContext ctx;
  printf("Initializing Okamoto-Uchiyama CPU (C++/GMP + OpenMP) with ~3072-bit "
         "modulus...\n");
  ou_init(&ctx, 3072, state);

  printf("Running benchmark with %d iterations on %d threads...\n", count,
         omp_get_max_threads());

  mpz_t *msgs = (mpz_t *)malloc(count * sizeof(mpz_t));
  mpz_t *ciphers = (mpz_t *)malloc(count * sizeof(mpz_t));
  mpz_t *decrypted = (mpz_t *)malloc(count * sizeof(mpz_t));

  for (int i = 0; i < count; i++) {
    mpz_inits(msgs[i], ciphers[i], decrypted[i], NULL);
    mpz_urandomm(msgs[i], state, ctx.p); // msg < p
  }

  double start = omp_get_wtime();

#pragma omp parallel
  {
    gmp_randstate_t thread_state;
    gmp_randinit_default(thread_state);
    gmp_randseed_ui(thread_state, time(NULL) ^ omp_get_thread_num());

#pragma omp for
    for (int i = 0; i < count; i++) {
      ou_encrypt(ciphers[i], msgs[i], &ctx, thread_state);
    }

    gmp_randclear(thread_state);
  }
  double end_enc = omp_get_wtime();

  double start_dec = omp_get_wtime();
#pragma omp parallel for
  for (int i = 0; i < count; i++) {
    ou_decrypt(decrypted[i], ciphers[i], &ctx);
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
