#include <map.h>
#include <io.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

extern GLuint tiles_texture;

map_t *map_load(const char *path, GLuint texture)
{
  printf("Loading map file %s\n", path);

  // malloc a map
  map_t *map = malloc(sizeof(map_t));

  // load file data
  uint8_t *data = io_read_file(path, "rb");

  // parse map data
  map->width    = data[0];
  map->height   = data[1];
  map->depth    = data[2];
  map->texture  = texture;
  
  // memcpy map data
  size_t size = map->width*map->height*map->depth;
  map->data   = malloc(size);
  memcpy(map->data, &data[3], size);

  // create some space for indices and vertices
  GLfloat *vertices[2]; 
  GLuint  *indices[2];
  for (int i=0; i<2; i++) {
    vertices[i] = malloc(sizeof(GLfloat)*(size*(5*40)));
    indices[i] = malloc(sizeof(GLuint)*(size*(6*5)));
  }

  bool success = true;

  // setup vertices, indices etc
  int sizes[2] = {0, 0}, *index = NULL;
  GLfloat *v_pointer = NULL;
  GLuint *i_pointer = NULL;
  for (int z=0; z<map->depth; z++) {
    for (int y=0; y<map->height; y++) {
      for (int x=0; x<map->width; x++) {

        // get the tile etc
        int tile = map_get_val(map, x, y, z);
        tile_type_e tile_type = map_get_type(map, tile);
        if (tile > 0 && tile_type != TILE_TYPE_NONE) {

          // set appropriate pointers
          if (tile_type == TILE_TYPE_SOLID) {
            v_pointer = vertices[0];
            i_pointer = indices[0];
            index = &sizes[0];
          } else if (tile_type == TILE_TYPE_TRANSPARENT) {
            v_pointer = vertices[1];
            i_pointer = indices[1];
            index = &sizes[1];
          } else {
            printf("Encountered unknown tile type %u\n", tile_type);
            success = false;
            goto exit;
          }

          // create the faces based on the given format in tiles array
          const char *str = map_get_string(map, tile);
          for (int s=0; s<strlen(str); s++) {
            char c = str[s];

            // top face
            int other_tile = map_get_val(map, x, y, z+1);
            if ((other_tile < 1 || map_get_type(map, other_tile) != tile_type) && (c == 't' || c == 'a'))
              map_add_face(&v_pointer[(*index)*(6*8)], &i_pointer[(*index)*6], 't', tile+s, x, y, z, index);
            
            // left face
            other_tile = map_get_val(map, x-1, y, z);
            if ((other_tile < 1 || map_get_type(map, other_tile) != tile_type) && (c == 's' || c == 'l' || c == 'a' || c == 'z'))
              map_add_face(&v_pointer[(*index)*(6*8)], &i_pointer[(*index)*6], 'l', tile+s, x, y, z, index);
            
            // right face
            other_tile = map_get_val(map, x+1, y, z);
            if ((other_tile < 1 || map_get_type(map, other_tile) != tile_type) && (c == 's' || c == 'r' || c == 'a' || c == 'z'))
              map_add_face(&v_pointer[(*index)*(6*8)], &i_pointer[(*index)*6], 'r', tile+s, x, y, z, index);
            
            // front face
            other_tile = map_get_val(map, x, y+1, z);
            if ((other_tile < 1 || map_get_type(map, other_tile) != tile_type) && (c == 's' || c == 'f' || c == 'a'))
              map_add_face(&v_pointer[(*index)*(6*8)], &i_pointer[(*index)*6], 'f', tile+s, x, y, z, index);
            
            // back face
            other_tile = map_get_val(map, x, y-1, z);
            if ((other_tile < 1 || map_get_type(map, other_tile) != tile_type) && (c == 's' || c == 'b' || c == 'a' || c == 'z'))
              map_add_face(&v_pointer[(*index)*(6*8)], &i_pointer[(*index)*6], 'b', tile+s, x, y, z, index);
          }
        }
      }
    }
  }

  // finally create the two meshs
  map->mesh[0] = mesh_new(vertices[0], sizes[0]*6*8, indices[0], sizes[0]*6, texture);
  map->mesh[1] = mesh_new(vertices[1], sizes[1]*6*8, indices[1], sizes[1]*6, texture);

  printf("Finished loading map %s\n", path);

exit:
  free(data);
  free(vertices[0]);
  free(vertices[1]);
  free(indices[0]);
  free(indices[1]);
  
  if (success)
    return map;

  printf("Failed to create map %s\n", path);
  free(map->data);
  free(map);
  return NULL;
}

int map_get_val(map_t *map, int x, int y, int z)
{
  // check bounds
  if (x < 0 || y < 0 || z < 0)
    return 0;

  if (x > map->width-1 || y > map->height-1 || z > map->depth-1)
    return 0;

  // index tile
  int i = (z*(map->width*map->height))+(y*map->width)+x;
  
  if (i > map->width*map->height*map->depth || i < 0)
    return 0;

  return map->data[i]-1;
}

tile_type_e map_get_type(map_t *map, int tile)
{
  if (tile < 0 || tile > TILE_FORMAT_SIZE)
    return TILE_TYPE_NONE;

  return tiles[tile].type;
}

const char *map_get_string(map_t *map, int tile)
{
  if (tile < 0 || tile > TILE_FORMAT_SIZE)
    return NULL;

  return (const char*)&tiles[tile].sides;
}

void map_destroy(map_t *map)
{
  mesh_destroy(map->mesh[0]);
  mesh_destroy(map->mesh[1]);
  free(map->data);
  free(map);
  map = NULL;
}

void map_add_face(GLfloat *vertices, GLuint *indices, char pos, int tile, int x, int y, int z, int *index)
{
  //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

  // add the indices and vertices
  switch (pos) {
    case 't': {
      memcpy(vertices, cube_top_vertices, sizeof(GLfloat)*(6*8));
      memcpy(indices, cube_top_indices, sizeof(GLuint)*6);
      break;
    };
    case 'l': {
      memcpy(vertices, cube_left_vertices, sizeof(GLfloat)*(6*8));
      memcpy(indices, cube_left_indices, sizeof(GLuint)*6);
      break;
    }
    case 'r': {
      memcpy(vertices, cube_right_vertices, sizeof(GLfloat)*(6*8));
      memcpy(indices, cube_right_indices, sizeof(GLuint)*6);
      break;
    }
    case 'f': {
      memcpy(vertices, cube_front_vertices, sizeof(GLfloat)*(6*8));
      memcpy(indices, cube_front_indices, sizeof(GLuint)*6);
      break;
    }
    case 'b': {
      memcpy(vertices, cube_back_vertices, sizeof(GLfloat)*(6*8));
      memcpy(indices, cube_back_indices, sizeof(GLuint)*6);
      break;
    }
  }

  // offset vertex positions and indices
  for (int i=0; i<6; i++) {
    // offset vertices
    vertices[(i*8)+0] += x;
    vertices[(i*8)+1] += z;
    vertices[(i*8)+2] += y;
    
    // offset indices
    indices[i] += (*index)*6;
    
    // offset tex coords
    int tile_count = floor(TILE_IMAGE_SIZE/TILE_SIZE);
    int tx = (tile-((tile/tile_count)*tile_count));
    int ty = (tile/tile_count);
    vertices[(i*8)+6] += ((TILE_SIZE+2.0f)/TILE_IMAGE_SIZE_PADDED)*tx;
    vertices[(i*8)+7] += ((TILE_SIZE+2.0f)/TILE_IMAGE_SIZE_PADDED)*ty;
  }

  (*index)++;
}