#ifndef SOUND_SAMPLES_H
#define SOUND_SAMPLES_H

#include <stdint.h>

typedef struct {
  const char    *name;
  const int16_t *samples;
  const uint32_t nb_samples;
} sound_sample_t;

#define NB_SOUND_SAMPLES 21

extern const sound_sample_t *const sound_samples[NB_SOUND_SAMPLES];

#endif /* SOUND_SAMPLES_H */
