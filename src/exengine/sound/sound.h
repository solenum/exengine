/* sound
  The sound module, handles
  loading and playing sound files.

  Based on OpenAL (the good one),
  still requires the ability to
  stream sound sources.  Currently
  also only supports ogg sound files.

  This needs a lot of work:
  * Decode other formats
  * Add source streaming
  * Ability to change individual sources values,
    such as if it should loop, the volume and
    position, etc.
*/

#ifndef EX_SOUND_H
#define EX_SOUND_H

#define EX_DEVICE_LEN 32

#include <inttypes.h>
#include "AL/al.h"
#include "AL/alc.h"

typedef enum {
  EX_SOURCE_STATIC    = 0,
  EX_SOURCE_STREAMING = 1,
  EX_SOURCE_LOOPING   = 1,
  EX_SOURCE_LEN
} ex_source_type_e;

typedef struct {
  ALCdevice *output, *input;
  ALCcontext *context;
} ex_sound_t;

typedef struct {
  ALuint id, buffers[3];
  ALint  ready_buffers[3];
  int looping, stopped;

  // for streaming source
  void *decoder;
  int streaming, channels, rate;
  size_t sample, samples;
  short *decode_buffer;
  size_t decode_buffer_bytes, decode_buffer_shorts;
} ex_source_t;

typedef enum {
  EX_SOUND_WAV,
  EX_SOUND_OGG,
} ex_sound_e;

extern ex_sound_t ex_sound;

typedef struct {
  ALCchar names[EX_DEVICE_LEN][512];
  size_t len;
} ex_sound_devices_t;

/**
 * [ex_sound_init init the sound module]
 */
void ex_sound_init();

/**
 * [ex_sound_list_devices description]
 * @param list  [description]
 * @param param [description]
 */
void ex_sound_list_devices(ex_sound_devices_t *list, const ALenum param);

/**
 * [ex_sound_set_output description]
 * @param device [description]
 */
void ex_sound_set_output(const ALCchar *device);

/**
 * [ex_sound_load_source load and decode a source into memory]
 * @param  path   [the sound file to load]
 * @param  loop   [1 if the sound is looping]
 * @return        [the new sound]
 */
ex_source_t* ex_sound_load(const char *path, int type, int looping);

/**
 * [ex_sound_restart restart a sound]
 * @param s [sound to restart]
 */
void ex_sound_restart(ex_source_t *s);

/**
 * [ex_sound_play play a sound source]
 * @param s [source to play]
 */
void ex_sound_play(ex_source_t *s);

/**
 * [ex_sound_destroy cleanup a sound source]
 * @param s [the source to destroy]
 */
void ex_sound_destroy(ex_source_t *s);

/**
 * [ex_sound_exit cleanup the sound module]
 */
void ex_sound_exit();

/**
 * [ex_sound_master_volume set the master volume]
 * @param vol [0.0 to 1.0]
 */
static inline void ex_sound_master_volume(float vol) {
  alListenerf(AL_GAIN, vol);
};

/**
 * [ex_sound_playing check if a source is playing]
 * @param  s [the source to check]
 * @return   [1 if playing]
 */
static inline int ex_sound_playing(ex_source_t *s) {
  ALenum state;
  alGetSourcei(s->id, AL_SOURCE_STATE, &state);
  return (state == AL_PLAYING);
}

#endif // EX_SOUND_H