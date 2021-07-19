/* cache
  Handles loading and caching most generic types
  of data.

  Once something has been loaded once, it will
  be kept in cache and future references to
  that file will instead return a copy of the
  cached data.

  Note that most data loading functions (like
  the iqm loader) call these internally already.
*/

#ifndef EX_CACHE_H
#define EX_CACHE_H

#include "glad/glad.h"
#include <SDL2/SDL.h>

#include "render/model.h"

/**
 * [ex_cache_init inits the cache module]
 *
 */
void ex_cache_init();

/**
 * [ex_cache_model store model in the cache]
 * @param m [model to add]
 */
void ex_cache_model(ex_model_t *m);

/**
 * [ex_cache_get_model get a copy of a model, if it exists in cache]
 * @param  path  [path to the model file]
 * @return       [a copy of the requested model]
 */
ex_model_t* ex_cache_get_model(const char *path);

/**
 * [ex_cache_texture store texture in cache and/or return cached texture]
 * @param  path [path to the texture file]
 * @return      [the cached texture ID]
 */
GLuint ex_cache_texture(const char *path);

/**
 * [ex_cache_flush cleanup all data from the cache]
 */
void ex_cache_flush();

#endif // EX_CACHE_H