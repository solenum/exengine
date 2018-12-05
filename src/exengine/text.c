#include "text.h"
#include "exe_io.h"
#include "mathlib.h"
#include <string.h>
#include <stdio.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define INF   -1e240
#define RANGE 1.0

float* ex_font_msdf(stbtt_fontinfo *font, uint8_t c, size_t w, size_t h);

ex_font_t* ex_font_load(const char *path)
{
  // load the ttf data
  size_t length;
  uint8_t *data = (uint8_t*)io_read_file(path, "rb", &length);
  if (!data) {
    printf("[TEXT] Failed generating font %s\n", path);
    return NULL;
  }

  // process the ttf data
  stbtt_fontinfo font;
  stbtt_InitFont(&font, (const uint8_t*)data, stbtt_GetFontOffsetForIndex(data,0));

  // generate a msdf bitmap
  float *bitmap = ex_font_msdf(&font, 'A', 16, 16);
  uint8_t *shitmap = malloc(16*16*3);
  for (int y=0; y<16; y++) {
    for (int x=0; x<16; x++) {
      size_t index = 3*((y*16)+x);
      printf("(%.1f %.1f %.1f)", fabs(bitmap[index]), fabs(bitmap[index+1]), fabs(bitmap[index+2]) );
      shitmap[index+0] = (uint8_t)(255/bitmap[index+0]);
      shitmap[index+1] = (uint8_t)(255/bitmap[index+1]);
      shitmap[index+2] = (uint8_t)(255/bitmap[index+2]);
    }
    printf("\n");
  }

  stbi_write_png("out.png", 16, 16, 3, shitmap, 3);

  free(data);
  printf("[TEXT] Done generating msdf font for %s\n", path);

  // ex_font_t *f = malloc(sizeof(ex_font_t));
  return NULL;
}

typedef struct {
  double dist;
  double d;
} signed_distance_t;

// defines what color channel an edge belongs to
enum EdgeColor {
    BLACK = 0,
    RED = 1,
    GREEN = 2,
    YELLOW = 3,
    BLUE = 4,
    MAGENTA = 5,
    CYAN = 6,
    WHITE = 7
};

// the possible types:
// STBTT_vmove  = start of a contour
// STBTT_vline  = linear segment
// STBTT_vcurve = quadratic segment
// STBTT_vcubic = cubic segment
typedef struct {
  int color;
  vec2 p[4];
  int type;
} edge_segment_t;

inline double cross(vec2 a, vec2 b)
{
  return a[0]*b[1] - a[1]*b[0];
}

inline int nonzero_sign(double n)
{
  return 2*(n > 0)-1;
}

int solve_quadratic(double x[2], double a, double b, double c)
{
    if (fabs(a) < 1e-14) {
        if (fabs(b) < 1e-14) {
            if (c == 0)
                return -1;
            return 0;
        }
        x[0] = -c/b;
        return 1;
    }
    double dscr = b*b-4*a*c;
    if (dscr > 0) {
        dscr = sqrt(dscr);
        x[0] = (-b+dscr)/(2*a);
        x[1] = (-b-dscr)/(2*a);
        return 2;
    } else if (dscr == 0) {
        x[0] = -b/(2*a);
        return 1;
    } else
        return 0;
}

int solve_cubic_normed(double *x, double a, double b, double c)
{
    double a2 = a*a;
    double q  = (a2 - 3*b)/9; 
    double r  = (a*(2*a2-9*b) + 27*c)/54;
    double r2 = r*r;
    double q3 = q*q*q;
    double A, B;
    if (r2 < q3) {
        double t = r/sqrt(q3);
        if (t < -1) t = -1;
        if (t > 1) t = 1;
        t = acos(t);
        a /= 3; q = -2*sqrt(q);
        x[0] = q*cos(t/3)-a;
        x[1] = q*cos((t+2*M_PI)/3)-a;
        x[2] = q*cos((t-2*M_PI)/3)-a;
        return 3;
    } else {
        A = -pow(fabs(r)+sqrt(r2-q3), 1/3.); 
        if (r < 0) A = -A;
        B = A == 0 ? 0 : q/A;
        a /= 3;
        x[0] = (A+B)-a;
        x[1] = -0.5*(A+B)-a;
        x[2] = 0.5*sqrt(3.)*(A-B);
        if (fabs(x[2]) < 1e-14)
            return 2;
        return 1;
    }
}

int solve_cubic(double x[3], double a, double b, double c, double d)
{
    if (fabs(a) < 1e-14)
        return solve_quadratic(x, b, c, d);
    return solve_cubic_normed(x, b/a, c/a, d/a);
}

void getortho(vec2 r, vec2 const v, int polarity, int allow_zero)
{
  double len = vec2_len(v);
  
  if (len == 0) {
    if (polarity) {
      r[0] = 0;
      r[1] = !allow_zero;
    } else {
      r[0] = 0;
      r[1] = -!allow_zero;
    }
    return;
  }
  
  if (polarity) {
    r[0] = -v[1]/len;
    r[1] = v[0]/len;
  } else {
    r[0] = v[1]/len;
    r[1] = -v[0]/len;
  }
}

void mix(vec2 r, vec2 a, vec2 b, double weight)
{ 
  r[0] = (1-weight)*a[0]+weight*b[0];
  r[1] = (1-weight)*a[1]+weight*b[1];
}

void linear_direction(vec2 r, edge_segment_t *e, double param)
{
  r[0] = e->p[1][0] - e->p[0][0];
  r[1] = e->p[1][1] - e->p[0][1];
}

void quadratic_direction(vec2 r, edge_segment_t *e, double param)
{
  vec2 a,b;
  vec2_sub(a, e->p[1], e->p[0]);
  vec2_sub(b, e->p[2], e->p[1]);
  mix(r, a, b, param);
}

void cubic_direction(vec2 r, edge_segment_t *e, double param)
{
  vec2 a,b,c,d,t;
  vec2_sub(a, e->p[1], e->p[0]);
  vec2_sub(b, e->p[2], e->p[1]);
  mix(c, a, b, param);
  vec2_sub(a, e->p[3], e->p[2]);
  mix(d, b, a, param);
  mix(t, c, d, param);

  if (!t[0] && !t[1]) {
    if (param == 0) {
      r[0] = e->p[2][0] - e->p[0][0];
      r[1] = e->p[2][1] - e->p[0][1];
      return;
    }
    if (param == 1) {
      r[0] = e->p[3][0] - e->p[1][0];
      r[1] = e->p[3][1] - e->p[1][1];
      return;
    }
  }

  r[0] = t[0];
  r[1] = t[1];
}

void direction(vec2 r, edge_segment_t *e, double param)
{
  switch (e->type) {
    case STBTT_vmove: {
      linear_direction(r, e, param);
      break;
    }
    case STBTT_vline: {
      linear_direction(r, e, param);
      break;
    }
    case STBTT_vcurve: {
      quadratic_direction(r, e, param);
      break;
    }
    case STBTT_vcubic: {
      cubic_direction(r, e, param);
      break;
    }
  }
}

void linear_point(vec2 r, edge_segment_t *e, double param)
{
  mix(r, e->p[0], e->p[1], param);
}

void quadratic_point(vec2 r, edge_segment_t *e, double param)
{
  vec2 a,b;
  mix(a, e->p[0], e->p[1], param);
  mix(b, e->p[1], e->p[2], param);
  mix(r, a, b, param);
}

void cubic_point(vec2 r, edge_segment_t *e, double param)
{
  vec2 p12, a, b, c, d;
  mix(p12, e->p[1], e->p[2], param);

  mix(a, e->p[0], e->p[1], param);
  mix(b, a, p12, param);

  mix(c, e->p[2], e->p[3], param);
  mix(d, p12, c, param);

  mix(r, b, d, param);
}

void point(vec2 r, edge_segment_t *e, double param)
{
  switch (e->type) {
    case STBTT_vmove: {
      linear_point(r, e, param);
      break;
    }
    case STBTT_vline: {
      linear_point(r, e, param);
      break;
    }
    case STBTT_vcurve: {
      quadratic_point(r, e, param);
      break;
    }
    case STBTT_vcubic: {
      cubic_point(r, e, param);
      break;
    }
  }
}

// linear edge signed distance 
signed_distance_t linear_dist(edge_segment_t *e, vec2 origin, double *param)
{
  vec2 aq, ab, eq;
  vec2_sub(aq, origin, e->p[0]);
  vec2_sub(ab, e->p[1], e->p[0]);
  *param = vec2_mul_inner(aq, ab) / vec2_mul_inner(ab, ab);
  vec2_sub(eq, e->p[*param > .5], origin);

  double endpoint_distance = vec2_len(eq);
  if (*param > 0 && *param < 1) {
    vec2 ab_ortho;
    getortho(ab_ortho, ab, 0 , 0);
    double ortho_dist = vec2_mul_inner(ab_ortho, aq);
    if (fabs(ortho_dist) < endpoint_distance)
      return (signed_distance_t){ortho_dist, 0};
  }

  vec2_norm(ab, ab);
  vec2_norm(eq, eq);
  double dist = nonzero_sign(cross(aq, ab)) * endpoint_distance;
  double d    = fabs(vec2_mul_inner(ab, eq));
  return (signed_distance_t){dist, d};
}

// quadratic edge signed distance
signed_distance_t quadratic_dist(edge_segment_t *e, vec2 origin, double *param)
{
  vec2 qa, ab, br;
  vec2_sub(qa, e->p[0], origin);
  vec2_sub(ab, e->p[1], e->p[0]);
  br[0] = e->p[0][0] + e->p[2][0] - e->p[1][0] - e->p[1][0];
  br[1] = e->p[0][1] + e->p[2][1] - e->p[1][1] - e->p[1][1];

  double a = vec2_mul_inner(br, br);
  double b = 3*vec2_mul_inner(ab, br);
  double c = 2*vec2_mul_inner(ab, ab)+vec2_mul_inner(qa, br);
  double d = vec2_mul_inner(qa, ab);
  double t[3];
  int solutions = solve_cubic(t, a, b, c, d);

  // distance from a
  double min_distance = nonzero_sign(cross(ab, qa))*vec2_len(qa);
  *param = -vec2_mul_inner(qa, ab)  / vec2_mul_inner(ab, ab);
  {
    vec2 a,b;
    vec2_sub(a, e->p[2], e->p[1]);
    vec2_sub(b, e->p[2], origin);

    // distance from b
    double distance = nonzero_sign(cross(a, b))*vec2_len(b);
    if (fabs(distance) < fabs(min_distance)) {
      min_distance = distance;

      vec2_sub(a, origin, e->p[1]);
      vec2_sub(b, e->p[2], e->p[1]);
      *param = vec2_mul_inner(a, b) / vec2_mul_inner(b, b);
    }
  }

  for (int i=0; i<solutions; ++i) {
    if (t[i] > 0 && t[i] < 1) {
      // end_point = p[0]+2*t[i]*ab+t[i]*t[i]*br;
      vec2 end_point, a, b;
      end_point[0] = e->p[0][0]+2*t[i]*ab[0]+t[i]*t[i]*br[0];
      end_point[1] = e->p[0][1]+2*t[i]*ab[1]+t[i]*t[i]*br[1];
      
      vec2_sub(a, e->p[2], e->p[0]);
      vec2_sub(b, end_point, origin);
      double distance = nonzero_sign(cross(a, b))*vec2_len(b);
      if (fabs(distance) <= fabs(min_distance)) {
        min_distance = distance;
        *param = t[i];
      }
    }
  }

  if (*param >= 0 && *param <= 1)
    return (signed_distance_t){min_distance, 0};

  vec2 aa,bb;
  vec2_norm(ab, ab);
  vec2_norm(qa, qa);
  vec2_sub(aa, e->p[2], e->p[1]);
  vec2_norm(aa, aa);
  vec2_sub(bb, e->p[2], origin);
  vec2_norm(bb, bb);

  if (*param < .5)
    return (signed_distance_t){min_distance, fabs(vec2_mul_inner(ab, qa))};
  else
    return (signed_distance_t){min_distance, fabs(vec2_mul_inner(aa, bb))};
}

// cubic edge signed distance
signed_distance_t cubic_dist(edge_segment_t *e, vec2 origin, double *param)
{
  vec2 qa, ab, br, as;
  vec2_sub(qa, e->p[0], origin);
  vec2_sub(ab, e->p[1], e->p[0]);
  br[0] = e->p[2][0] - e->p[1][0] - ab[0];
  br[1] = e->p[2][1] - e->p[1][1] - ab[1];
  as[0] = (e->p[3][0] - e->p[2][0]) - (e->p[2][0] - e->p[1][0]) - br[0];
  as[1] = (e->p[3][1] - e->p[2][1]) - (e->p[2][1] - e->p[1][1]) - br[1];

  vec2 ep_dir;
  direction(ep_dir, e, 0);

  // distance from a
  double min_distance = nonzero_sign(cross(ep_dir, qa))*vec2_len(qa);
  *param = -vec2_mul_inner(qa, ep_dir)  / vec2_mul_inner(ep_dir, ep_dir);
  {
    vec2 a;
    vec2_sub(a, e->p[3], origin);

    direction(ep_dir, e, 1);
    // distance from b
    double distance = nonzero_sign(cross(ep_dir, a))*vec2_len(a);
    if (fabs(distance) < fabs(min_distance)) {
      min_distance = distance;

      a[0] = origin[0] + ep_dir[0] - e->p[3][0];
      a[1] = origin[1] + ep_dir[1] - e->p[3][1];
      *param = vec2_mul_inner(a, ep_dir) / vec2_mul_inner(ep_dir, ep_dir);
    }
  }

  const int search_starts = 4;
  for (int i=0; i<=search_starts; ++i) {
    double t = (double)i/search_starts;
    for (int step=0;; ++step) {
      vec2 qpt;
      point(qpt, e, t);
      vec2_sub(qpt, qpt, origin);
      vec2 d;
      direction(d, e, t);
      double distance = nonzero_sign(cross(d, qpt))*vec2_len(qpt);
      if (fabs(distance) < fabs(min_distance)) {
        min_distance = distance;
        *param = t;
      }
      if (step == search_starts)
        break;
      
      vec2 d1,d2;
      d1[0] = 3*as[0]*t*t+6*br[0]*t+3*ab[0];
      d1[1] = 3*as[1]*t*t+6*br[1]*t+3*ab[1];
      d2[0] = 6*as[0]*t+6*br[0];
      d2[1] = 6*as[1]*t+6*br[1];

      t -= vec2_mul_inner(qpt, d1) / (vec2_mul_inner(d1, d1)+vec2_mul_inner(qpt, d2));
      if (t < 0 || t > 1)
        break;
    }
  }

  if (*param >= 0 && *param <= 1)
    return (signed_distance_t){min_distance, 0};

  vec2 d0, d1;
  direction(d0, e, 0);
  direction(d1, e, 1);
  vec2_norm(d0, d0);
  vec2_norm(d1, d1);
  vec3_norm(qa, qa);
  vec2 a;
  vec2_sub(a, e->p[3], origin);
  vec2_norm(a, a);

  if (*param < .5)
    return (signed_distance_t){min_distance, fabs(vec2_mul_inner(d0, qa))};
  else
    return (signed_distance_t){min_distance, fabs(vec2_mul_inner(d1, a))};
}

void dist_to_pseudo(signed_distance_t *distance, vec2 origin, double param, edge_segment_t *e)
{
  if (param < 0) {
    vec2 dir;
    direction(dir, e, 0);
    vec2_norm(dir, dir);
    vec2 aq  = {origin[0], origin[1]};
    double ts = vec2_mul_inner(aq, dir);
    if (ts < 0) {
      double pseudo_dist = cross(aq, dir);
      if (fabs(pseudo_dist) <= fabs(distance->dist)) {
        distance->dist = pseudo_dist;
        distance->d    = 0;
      }
    }
  } else if (param > 1) {
    vec2 dir;
    direction(dir, e, 1);
    vec2_norm(dir, dir);
    vec2 bq  = {origin[0]-1, origin[1]-1};
    double ts = vec2_mul_inner(bq, dir);
    if (ts > 0) {
      double pseudo_dist = cross(bq, dir);
      if (fabs(pseudo_dist) <= fabs(distance->dist)) {
        distance->dist = pseudo_dist;
        distance->d    = 0;
      }
    }
  }
}

int signed_compare(signed_distance_t a, signed_distance_t b)
{
  return fabs(a.dist) < fabs(b.dist) || (fabs(a.dist) == fabs(b.dist) && a.d < b.d);
}

double median(double a, double b, double c)
{
  return MAX(MIN(a, b), MIN(MAX(a, b), c));
}


float* ex_font_msdf(stbtt_fontinfo *font, uint8_t c, size_t w, size_t h)
{
  float *bitmap = malloc(sizeof(float)*3*w*h);
  memset(bitmap, 0, sizeof(float)*3*w*h);

  stbtt_vertex *verts;
  int num_verts = stbtt_GetGlyphShape(font, c, &verts);
  
  int winding_count      = 0;
  int *winding_lengths   = NULL;
  stbtt__point *windings = stbtt_FlattenCurves(verts, num_verts, 1, &winding_lengths, &winding_count, 0);
  
  printf("[TEXT] Number of contours for '%c': %i\n", c, winding_count);
  printf("[TEXT] Number of verts for '%c': %i\n", c, num_verts);

  // figure out how many contours exist
  int contour_count = 0;
  for (int i=0; i<num_verts; i++) {
    if (verts[i].type == STBTT_vmove)
      contour_count++;
  }

  // determin what vertices belong to what contours
  typedef struct {
    size_t start,end;
  } indices_t;
  indices_t *contours = malloc(sizeof(indices_t) * contour_count);
  int j = 0;
  for (int i=0; i<=num_verts; i++) {
    if (verts[i].type == STBTT_vmove) {
      if (i > 0) {
        contours[j].end = i;
        j++;
      }

      contours[j].start = i;
    } else if (i >= num_verts) {
      contours[j].end = i;
    }
  }

  typedef struct {
    signed_distance_t min_distance;
    edge_segment_t *near_edge;
    double near_param;
  } edge_point_t;

  typedef struct {
    edge_segment_t *edges;
    size_t edge_count;
  } contour_t;
  
  // process verts into series of contour-specific edge lists
  vec2 initial = {0, 0}; // fix this?
  contour_t *contour_data = malloc(sizeof(contour_t) * contour_count);
  for (int i=0; i<contour_count; i++) {
    size_t count = contours[i].end - contours[i].start;
    contour_data[i].edges = malloc(sizeof(edge_segment_t) * count);
    contour_data[i].edge_count = 0;

    size_t k = 0;
    for (int j=contours[i].start; j<contours[i].end; j++) {

      edge_segment_t *e = &contour_data[i].edges[k++];
      stbtt_vertex *v   = &verts[j];
      e->type  = v->type;
      e->color = WHITE;

      switch (v->type) {
        case STBTT_vmove: {
          vec2 p = {v->x/64.0, v->y/64.0};
          memcpy(&initial, p, sizeof(vec2));
          break;
        }

        case STBTT_vline: {
          vec2 p = {v->x/64.0, v->y/64.0};
          memcpy(&e->p[0], initial, sizeof(vec2));
          memcpy(&e->p[1], p, sizeof(vec2));
          memcpy(&initial, p, sizeof(vec2));
          contour_data->edge_count++;
          break;
        }

        case STBTT_vcurve: {
          vec2 p = {v->x/64.0, v->y/64.0};
          vec2 c = {v->cx/64.0, v->cy/64.0};
          memcpy(&e->p[0], initial, sizeof(vec2));
          memcpy(&e->p[1], c, sizeof(vec2));
          memcpy(&e->p[2], p, sizeof(vec2));
          
          if ((e->p[0][0] == e->p[1][0] && e->p[0][1] == e->p[1][1]) ||
              (e->p[1][0] == e->p[2][0] && e->p[1][1] == e->p[2][1])) { 
            e->p[1][0] = 0.5*(e->p[0][0]+e->p[2][0]);
            e->p[1][1] = 0.5*(e->p[0][1]+e->p[2][1]);
          }

          memcpy(&initial, p, sizeof(vec2));
          contour_data->edge_count++;
          break;
        }

        case STBTT_vcubic: {
          vec2 p = {v->x/64.0, v->y/64.0};
          vec2 c = {v->cx/64.0, v->cy/64.0};
          vec2 c1 = {v->cx1/64.0, v->cy1/64.0};
          memcpy(&e->p[0], initial, sizeof(vec2));
          memcpy(&e->p[1], c, sizeof(vec2));
          memcpy(&e->p[2], c1, sizeof(vec2));
          memcpy(&e->p[3], p, sizeof(vec2));
          memcpy(&initial, p, sizeof(vec2));
          contour_data->edge_count++;
          break;
        }
      }
    }
  }

  typedef struct {
    double r, g, b;
    double med;
  } multi_distance_t;

  multi_distance_t *contour_sd;
  contour_sd = malloc(sizeof(multi_distance_t) * contour_count);

  for (int y=0; y<h; y++) {
    for (int x=0; x<w; x++) {
      vec2 p = {(x+.5), (y+.5)};

      edge_point_t sr, sg, sb;
      sr.near_edge = sg.near_edge = sb.near_edge = NULL;
      sr.near_param = sg.near_param = sb.near_param = 0;
      sr.min_distance.dist = sg.min_distance.dist = sb.min_distance.dist = INF;
      double d = fabs(INF);
      double neg_dist = -INF;
      double pos_dist = INF;
      int winding = 0;

      for (int j=0; j<contour_count; ++j) {
        edge_point_t r, g, b;
        r.near_edge = g.near_edge = b.near_edge = NULL;
        r.near_param = g.near_param = b.near_param = 0;
        r.min_distance.dist = g.min_distance.dist = b.min_distance.dist = INF;

        for (int k=0; k<contour_data[j].edge_count; ++k) {
          edge_segment_t *e = &contour_data[j].edges[k];
          double param;
          signed_distance_t distance;

          // calculate signed distance
          switch (e->type) {
            case STBTT_vline: {
              distance = linear_dist(e, p, &param);
              break;
            }
            case STBTT_vcurve: {
              distance = quadratic_dist(e, p, &param);
              break;
            }
            case STBTT_vcubic: {
              distance = cubic_dist(e, p, &param);
              break;
            }
          }

          if (e->color&RED && signed_compare(distance, r.min_distance)) {
            r.min_distance = distance;
            r.near_edge    = e;
            r.near_param   = param;
          }
          if (e->color&GREEN && signed_compare(distance, g.min_distance)) {
            g.min_distance = distance;
            g.near_edge    = e;
            g.near_param   = param;
          }
          if (e->color&BLUE && signed_compare(distance, b.min_distance)) {
            b.min_distance = distance;
            b.near_edge    = e;
            b.near_param   = param;
          }
        }

        if (signed_compare(r.min_distance, sr.min_distance))
          sr = r;
        if (signed_compare(g.min_distance, sg.min_distance))
          sg = g;
        if (signed_compare(b.min_distance, sb.min_distance))
          sb = b;

        double med_min_dist = fabs(median(r.min_distance.dist, g.min_distance.dist, b.min_distance.dist));

        int w = stbtt__compute_crossings_x(p[0], p[1], num_verts, verts);
        if (med_min_dist < d) {
          d = med_min_dist;
          winding = -w;
        }

        if (r.near_edge)
          dist_to_pseudo(&r.min_distance, p, r.near_param, r.near_edge);
        if (g.near_edge)
          dist_to_pseudo(&g.min_distance, p, g.near_param, g.near_edge);
        if (b.near_edge)
          dist_to_pseudo(&b.min_distance, p, b.near_param, b.near_edge);

        med_min_dist = median(r.min_distance.dist, g.min_distance.dist, b.min_distance.dist);
        contour_sd[j].r = r.min_distance.dist;
        contour_sd[j].g = g.min_distance.dist;
        contour_sd[j].b = b.min_distance.dist;
        contour_sd[j].med = med_min_dist;

        if (w > 0 && med_min_dist >= 0 && fabs(med_min_dist) < fabs(pos_dist))
          pos_dist = med_min_dist;
        if (w < 0 && med_min_dist <= 0 && fabs(med_min_dist) < fabs(neg_dist))
          neg_dist = med_min_dist;
      }

      if (sr.near_edge)
        dist_to_pseudo(&sr.min_distance, p, sr.near_param, sr.near_edge);
      if (sg.near_edge)
        dist_to_pseudo(&sg.min_distance, p, sg.near_param, sg.near_edge);
      if (sb.near_edge)
        dist_to_pseudo(&sb.min_distance, p, sb.near_param, sb.near_edge);
    
      multi_distance_t msd;
      msd.r = msd.g = msd.b = msd.med = INF;
      if (pos_dist >= 0 && fabs(pos_dist) <= fabs(neg_dist)) {
        msd.med = INF;
        winding = 1;
        for (int i=0; i<contour_count; ++i)
          if (w > 0 && contour_sd[i].med > msd.med && fabs(contour_sd[i].med) < fabs(neg_dist))
            msd = contour_sd[i];
      } else if (neg_dist <= 0 && fabs(neg_dist) <= fabs(pos_dist)) {
        msd.med = -INF;
        winding = -1;
        for (int i=0; i<contour_count; ++i)
          if (w < 0 && contour_sd[i].med < msd.med && fabs(contour_sd[i].med) < fabs(pos_dist))
            msd = contour_sd[i];
      }

      for (int i=0; i<contour_count; ++i)
        if (w != winding && fabs(contour_sd[i].med) < fabs(msd.med))
          msd = contour_sd[i];

      if (median(sr.min_distance.dist, sg.min_distance.dist, sb.min_distance.dist) == msd.med) {
        msd.r = sr.min_distance.dist;
        msd.g = sg.min_distance.dist;
        msd.b = sb.min_distance.dist;
      }

      size_t index = 3*((y*w)+x);
      bitmap[index] = (float)msd.r/RANGE+.5;   // r
      bitmap[index+1] = (float)msd.g/RANGE+.5; // g
      bitmap[index+2] = (float)msd.b/RANGE+.5; // b
    }
  }

  return bitmap;
}