#ifndef MAP_H
#define MAP_H

#include <mesh.h>
#include <inttypes.h>

// no idea to be honest
#define TILE_SIZE 16.0f
#define TILE_IMAGE_SIZE 512.0f
#define TILE_IMAGE_SIZE_PADDED 576.0f

// no touching pls
#define UV_ORIGIN (1.0f/TILE_IMAGE_SIZE_PADDED)
#define UV_OFFSET ((TILE_SIZE+1.0f)/TILE_IMAGE_SIZE_PADDED)
#define TILE_FORMAT_SIZE sizeof tiles / sizeof tiles[0]

typedef struct {
  uint32_t width, height, depth;
  uint8_t *data;
  mesh_t *mesh[2];
  GLuint texture;
} map_t;

typedef enum {
  TILE_TYPE_NONE = 0,
  TILE_TYPE_SOLID,
  TILE_TYPE_TRANSPARENT
} tile_type_e;

typedef struct {
  tile_type_e type;
  char sides[5];
} tile_format_t;

/**
 * tile cube map format array, string specifies textures in order
 * t = top, s = sides, l = left, r = right, f = front, b = back,
 * a = all, x = x axis, y = y axis, z = (left,right,back)
 */
static tile_format_t tiles[] = {
  {TILE_TYPE_NONE ,       '\0'},
  {TILE_TYPE_SOLID,       "ts\0"},
  {TILE_TYPE_NONE ,       '\0'},
  {TILE_TYPE_SOLID,       "ts\0"},
  {TILE_TYPE_NONE ,       '\0'},
  {TILE_TYPE_SOLID,       "ts\0"},
  {TILE_TYPE_NONE ,       '\0'},
  {TILE_TYPE_SOLID,       "a\0"},
  {TILE_TYPE_SOLID,       "a\0"},
  {TILE_TYPE_SOLID,       "a\0"},
  {TILE_TYPE_SOLID,       "tzf\0"},
  {TILE_TYPE_NONE ,       '\0'},
  {TILE_TYPE_NONE ,       '\0'},
  {TILE_TYPE_TRANSPARENT, "a\0"},
  {TILE_TYPE_TRANSPARENT, "a\0"},
};

/**
 * [map_load load a .map file and generate a mesh]
 * @param  path [file path to .map file]
 * @return      [pointer to malloc'd map_t]
 */
map_t *map_load(const char *path, GLuint texture);

/**
 * [map_get_val returns the tile value in a map (z is depth! not y)]
 * @param  map [map_t pointer]
 * @param  x   [x position]
 * @param  y   [y position]
 * @param  z   [z position]
 * @return     [tile value]
 */
int map_get_val(map_t *map, int x, int y, int z);

/**
 * [map_get_type returns the enum value of a given tile index]
 * @param  map  [map_t pointer]
 * @param  tile [tile index]
 * @return      [tile_type_e value]
 */
tile_type_e map_get_type(map_t *map, int tile);

/**
 * [map_get_string gets the string value of a given tile index]
 * @param  map  [map_t pointer]
 * @param  tile [tile index]
 * @return      [char string of format struct array]
 */
const char *map_get_string(map_t *map, int tile);

/**
 * [map_destroy free any malloc'd data]
 * @param map [map_t pointer]
 */
void map_destroy(map_t *map);

/**
 * [map_add_face adds vertices and indices for a given face]
 * @param vertices [pointer to vertices (offset it!)]
 * @param indices  [pointer to indices (again, offset it!)]
 * @param pos      [what face to generate, 't', 'l', 'r', 'f', 'b']
 * @param tile     [tile value for tex coords]
 * @param x        [x position]
 * @param y        [y position]
 * @param z        [z position (z is depth!)]
 * @param index    [index to offset indices value]
 */
void map_add_face(GLfloat *vertices, GLuint *indices, char pos, int tile, int x, int y, int z, int *index);

/**
 * The data for cube faces and indices
 */
static GLfloat cube_top_vertices[] = {
  0.0f,  1.0f, 0.0f,  0.0f, 1.0f, 0.0f, UV_ORIGIN,  UV_OFFSET,
  1.0f,  1.0f, 0.0f,  0.0f, 1.0f, 0.0f, UV_OFFSET,  UV_OFFSET,
  1.0f,  1.0f, 1.0f,  0.0f, 1.0f, 0.0f, UV_OFFSET,  UV_ORIGIN,
  1.0f,  1.0f, 1.0f,  0.0f, 1.0f, 0.0f, UV_OFFSET,  UV_ORIGIN,
  0.0f,  1.0f, 1.0f,  0.0f, 1.0f, 0.0f, UV_ORIGIN,  UV_ORIGIN,
  0.0f,  1.0f, 0.0f,  0.0f, 1.0f, 0.0f, UV_ORIGIN,  UV_OFFSET
};

static GLuint cube_top_indices[] = {
  0,2,1,3,5,4
};

static GLfloat cube_left_vertices[] = {
  0.0f,  1.0f, 1.0f,  -1.0f, 0.0f, 0.0f, UV_ORIGIN,  UV_ORIGIN,
  0.0f,  1.0f, 0.0f,  -1.0f, 0.0f, 0.0f, UV_OFFSET,  UV_ORIGIN,
  0.0f,  0.0f, 0.0f,  -1.0f, 0.0f, 0.0f, UV_OFFSET,  UV_OFFSET,
  0.0f,  0.0f, 0.0f,  -1.0f, 0.0f, 0.0f, UV_OFFSET,  UV_OFFSET,
  0.0f,  0.0f, 1.0f,  -1.0f, 0.0f, 0.0f, UV_ORIGIN,  UV_OFFSET,
  0.0f,  1.0f, 1.0f,  -1.0f, 0.0f, 0.0f, UV_ORIGIN,  UV_ORIGIN
};

static GLuint cube_left_indices[] = {
  0,1,2,3,4,5
};

static GLfloat cube_right_vertices[] = {
  1.0f,  1.0f, 1.0f,  1.0f, 0.0f, 0.0f, UV_ORIGIN,  UV_ORIGIN,
  1.0f,  1.0f, 0.0f,  1.0f, 0.0f, 0.0f, UV_OFFSET,  UV_ORIGIN,
  1.0f,  0.0f, 0.0f,  1.0f, 0.0f, 0.0f, UV_OFFSET,  UV_OFFSET,
  1.0f,  0.0f, 0.0f,  1.0f, 0.0f, 0.0f, UV_OFFSET,  UV_OFFSET,
  1.0f,  0.0f, 1.0f,  1.0f, 0.0f, 0.0f, UV_ORIGIN,  UV_OFFSET,
  1.0f,  1.0f, 1.0f,  1.0f, 0.0f, 0.0f, UV_ORIGIN,  UV_ORIGIN
};

static GLuint cube_right_indices[] = {
  2,1,0,5,4,3
};

static GLfloat cube_front_vertices[] = {
  0.0f,  0.0f, 1.0f,  0.0f, 0.0f, 1.0f, UV_ORIGIN,  UV_OFFSET,
  1.0f,  0.0f, 1.0f,  0.0f, 0.0f, 1.0f, UV_OFFSET,  UV_OFFSET,
  1.0f,  1.0f, 1.0f,  0.0f, 0.0f, 1.0f, UV_OFFSET,  UV_ORIGIN,
  1.0f,  1.0f, 1.0f,  0.0f, 0.0f, 1.0f, UV_OFFSET,  UV_ORIGIN,
  0.0f,  1.0f, 1.0f,  0.0f, 0.0f, 1.0f, UV_ORIGIN,  UV_ORIGIN,
  0.0f,  0.0f, 1.0f,  0.0f, 0.0f, 1.0f, UV_ORIGIN,  UV_OFFSET
};

static GLuint cube_front_indices[] = {
  0,1,2,3,4,5
};

static GLfloat cube_back_vertices[] = {
  0.0f,  0.0f, 0.0f,  0.0f, 0.0f, -1.0f, UV_ORIGIN,  UV_OFFSET,
  1.0f,  0.0f, 0.0f,  0.0f, 0.0f, -1.0f, UV_OFFSET,  UV_OFFSET,
  1.0f,  1.0f, 0.0f,  0.0f, 0.0f, -1.0f, UV_OFFSET,  UV_ORIGIN,
  1.0f,  1.0f, 0.0f,  0.0f, 0.0f, -1.0f, UV_OFFSET,  UV_ORIGIN,
  0.0f,  1.0f, 0.0f,  0.0f, 0.0f, -1.0f, UV_ORIGIN,  UV_ORIGIN,
  0.0f,  0.0f, 0.0f,  0.0f, 0.0f, -1.0f, UV_ORIGIN,  UV_OFFSET
};

static GLuint cube_back_indices[] = {
  2,1,0,5,4,3
};

#endif