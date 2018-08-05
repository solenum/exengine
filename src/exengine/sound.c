#include <stdio.h>
#include <stdlib.h>
#include <physfs.h>
#include "exe_io.h"
#include "sound.h"
#include "stb_vorbis.c"

ex_sound_t ex_sound;

void ex_sound_init()
{
  // init device
  ex_sound.device = alcOpenDevice(NULL);
  if (!ex_sound.device) {
    printf("Failed opening OpenAL device\n");
    return;
  }
  
  // init and set context
  ex_sound.context = alcCreateContext(ex_sound.device, NULL);
  if (!alcMakeContextCurrent(ex_sound.context)) {
    printf("Failed creating OpenAL context current\n");
    return;
  }

  // setup listener properties
  ALfloat pos[] = {0.0, 0.0, 1.0};
  ALfloat vel[] = {0.0, 0.0, 0.0};
  ALfloat ori[] = {0.0, 0.0, -1.0, 0.0, 1.0, 0.0};
  alListenerfv(AL_POSITION, pos);
  alListenerfv(AL_VELOCITY, vel);
  alListenerfv(AL_ORIENTATION, ori);
  alListenerf(AL_GAIN, 1.0);
}

ex_source_t* ex_sound_load_source(const char *path, ex_sound_e format, int loop)
{
  printf("Loading audio file %s\n", path);
  int channels, rate;
  short *data = NULL;
  int32_t decode_len = 0;

  // decode ogg data
  if (format == EX_SOUND_OGG) {
    printf("Decoding ogg format\n");
    size_t len = 0;
    uint8_t *file_data = io_read_file(path, "rb", &len);

    decode_len = stb_vorbis_decode_memory(file_data, len, &channels, &rate, &data);
    
    // loading failed
    if (decode_len <= 0) {
      printf("Failed decoding ogg file %s\n", path);
      return NULL;
    }

  }

  // init the al source
  ex_source_t *s = malloc(sizeof(ex_source_t));
  alGenSources(1, &s->id);

  // set default source values
  alSourcef(s->id, AL_PITCH, 1);
  alSourcef(s->id, AL_GAIN, 1);
  alSource3f(s->id, AL_POSITION, 0, 0, 0);
  alSource3f(s->id, AL_VELOCITY, 0, 0, 0);
  alSourcei(s->id, AL_LOOPING, loop);

  // buffer
  uint32_t length = decode_len * channels * (sizeof(int16_t) / sizeof(uint8_t));
  alGenBuffers(1, &s->buffer);
  alBufferData(s->buffer, AL_FORMAT_STEREO16, data, length, rate);

  // bind buffer to source
  alSourcei(s->id, AL_BUFFER, s->buffer);

  free(data);

  return s;
}

void ex_sound_destroy(ex_source_t *s)
{
  alDeleteSources(1, &s->id);
  alDeleteBuffers(1, &s->buffer);
  free(s);
  s = NULL;
}

void ex_sound_exit()
{
  alcMakeContextCurrent(NULL);
  alcDestroyContext(ex_sound.context);
  alcCloseDevice(ex_sound.device);
}