#include <stdio.h>
#include <stdlib.h>
#include <physfs.h>
#include "util/io.h"
#include "sound/sound.h"
#include "sound/stb_vorbis.c"

ex_sound_t ex_sound;
ex_sound_devices_t *ex_sound_outputs = NULL;
ex_sound_devices_t *ex_sound_inputs = NULL;

void ex_sound_init()
{
  // reset AL error stack
  alGetError();

  // get a list of available devices
  ex_sound_outputs = malloc(sizeof(ex_sound_devices_t));
  ex_sound_inputs = malloc(sizeof(ex_sound_devices_t));
  ex_sound_list_devices(ex_sound_outputs, ALC_DEVICE_SPECIFIER);
  ex_sound_list_devices(ex_sound_inputs, ALC_CAPTURE_DEVICE_SPECIFIER);

  // print out available devices
  for (int i=0; i<ex_sound_outputs->len; i++)
    printf("Output Device [%s]\n", ex_sound_outputs->names[i]);
  for (int i=0; i<ex_sound_inputs->len; i++)
    printf("Input Device [%s]\n", ex_sound_inputs->names[i]);

  // set default device as output
  ex_sound_set_output(NULL);
  
  // init and set context
  ex_sound.context = alcCreateContext(ex_sound.output, NULL);
  if (!alcMakeContextCurrent(ex_sound.context)) {
    printf("Failed creating OpenAL context\n");
    return;
  }

  // setup default listener properties
  ALfloat pos[] = {0.0, 0.0, 1.0};
  ALfloat vel[] = {0.0, 0.0, 0.0};
  ALfloat ori[] = {0.0, 0.0, -1.0, 0.0, 1.0, 0.0};
  alListenerfv(AL_POSITION, pos);
  alListenerfv(AL_VELOCITY, vel);
  alListenerfv(AL_ORIENTATION, ori);
  alListenerf(AL_GAIN, 1.0);
}

void ex_sound_list_devices(ex_sound_devices_t *list, const ALenum param)
{
  // clear device list
  list->len = 0;
  for (int i=0; i<EX_DEVICE_LEN; i++)
    list->names[i][0] = '\0';

  // get NULL separated strings of devices
  const ALCchar *devices = alcGetString(NULL, param);
  const ALCchar *device = devices, *next = devices + 1;

  // add devices to the list
  while (device && *device != '\0' && next && *next != '\0') {
    strcpy(&list->names[list->len++][0], device);
    device += (strlen(device) + 1);
    next += (strlen(device) + 2);
  }
}

void ex_sound_set_output(const ALCchar *device)
{
  // set default device
  if (!device) {
    printf("Output device set as default\n");
    ex_sound.output = alcOpenDevice(NULL);
    return;
  }

  // attempt to set requested device
  ex_sound.output = alcOpenDevice(device);

  // check for success 
  ALCenum error;
  error = alGetError();
  if (error != AL_NO_ERROR) {
    printf("Error setting audio output device %s\n", device);
  }

  // success
  printf("Output device set as %s\n", device);
}

ex_source_t* ex_sound_load(const char *path, int loop)
{
  printf("Loading audio file %s\n", path);
  int channels, rate;
  short *data = NULL;
  int32_t decode_len = 0;

  // get file extension
  char buf[512];
  ex_io_get_extension(buf, path);

  // decode ogg data
  if (strcmp(buf, ".ogg") == 0) {
    printf("Decoding ogg format\n");
    size_t len = 0;
    uint8_t *file_data = (uint8_t*)ex_io_read(path, "rb", &len);

    decode_len = stb_vorbis_decode_memory(file_data, len, &channels, &rate, &data);

    free(file_data);
    
    // loading failed
    if (decode_len <= 0) {
      printf("Failed decoding ogg file %s\n", path);
      return NULL;
    }
  } else {
    printf("Failed loading sound file %s\n", path);
    printf("Format \"%s\" is not currently supported\n", buf);
    return NULL;
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
  alcCloseDevice(ex_sound.output);
}