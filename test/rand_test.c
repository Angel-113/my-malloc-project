#include "../include/rand_test.h"
#include "rand.h"
#include <math.h>

pcg32_random_t init_prng(u64 initstate, u64 initseq) {
  pcg32_random_t prng = (pcg32_random_t){0};
  pcg32_srandom_r(&prng, initstate, initseq);
  return prng;
}

u32 randint32(pcg32_random_t *prng) { return pcg32_random_r(prng); }

u32 randint32_bounded(pcg32_random_t *prng, u32 bound) {
  return pcg32_boundedrand(bound);
}

u64 randint64(pcg32_random_t *prng) {
  return ldexpl(pcg32_random_r(prng), -32);
}
