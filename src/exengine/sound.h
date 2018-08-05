#ifndef EX_SOUND_H
#define EX_SOUND_H

#include <inttypes.h>
#include <AL/al.h>
#include <AL/alc.h>

typedef struct {
  ALCdevice *device;
  ALCcontext *context;
} ex_sound_t;

typedef struct {
  ALuint id, buffer;
} ex_source_t;

typedef enum {
  EX_SOUND_WAV,
  EX_SOUND_OGG,
} ex_sound_e;

extern ex_sound_t ex_sound;

void ex_sound_init();

ex_source_t* ex_sound_load_source(const char *path, ex_sound_e format, int loop);

void ex_sound_destroy(ex_source_t *s);

void ex_sound_exit();

static inline void ex_sound_master_volume(float vol) {
  alListenerf(AL_GAIN, vol);
};

static inline int ex_sound_playing(ex_source_t *s) {
  ALenum state;
  alGetSourcei(s->id, AL_SOURCE_STATE, &state);
  return (state == AL_PLAYING);
}

#endif // EX_SOUND_H