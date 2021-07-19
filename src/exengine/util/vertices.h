#ifndef EX_VERTICES_H
#define EX_VERTICES_H

#define EX_VERTICES_CUBE_LEN 24
static float ex_vertices_cube[] = {
  0.0f, 0.0f, 0.0f,
  1.0f, 0.0f, 0.0f,
  1.0f, 1.0f, 0.0f,
  0.0f, 1.0f, 0.0f,

  0.0f, 0.0f, 1.0f,
  1.0f, 0.0f, 1.0f,
  1.0f, 1.0f, 1.0f,
  0.0f, 1.0f, 1.0f,
};

#define EX_INDICES_CUBE_LEN 24
static unsigned int ex_indices_cube[] = {
  0, 1, 1, 2, 2, 3, 3, 0,
  4, 5, 5, 6, 6, 7, 7, 4,
  0, 4, 3, 7,
  1, 5, 2, 6
};

#endif // EX_VERTICES_H