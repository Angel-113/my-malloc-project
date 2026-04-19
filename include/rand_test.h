#ifndef RAND_TEST_H
#define RAND_TEST_H

#include "base.h"
#include "rand.h"

extern pcg32_random_t init_prng(u64 initstate, u64 initseq);
extern u32 randint32(pcg32_random_t *prng);
extern u64 randint64(pcg32_random_t *prng);
extern u32 randint32_bounded(pcg32_random_t *prng, u32 bound);

#endif
