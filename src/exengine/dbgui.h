#ifndef EX_DBGUI_H
#define EX_DBGUI_H

#include "glimgui.h"

typedef enum {
  ex_dbgprofiler_lighting_depth,
  ex_dbgprofiler_lighting_render,
  ex_dbgprofiler_update,
  ex_dbgprofiler_other,
  ex_dbgprofiler_count
} ex_dbgprofiler_e;

const static char *ex_dbgprofiler_strings[] = {
  "depth",
  "light",
  "updat",
  "other"
};

const static char *ex_dbgprofiler_strings_full[] = {
  "light depth",
  "light render",
  "scene update",
  "other non-render"
};

typedef struct {
  double begin[ex_dbgprofiler_count];
  double end[ex_dbgprofiler_count];
  double values[ex_dbgprofiler_count];
  double delta_begin, delta_end, delta_time, timer;
  float  frame_times[128];
  int    paused, last_frame_time, render_octree, octree_obj_only, wireframe;
} ex_dbgprofiler_t;

extern ex_dbgprofiler_t ex_dbgprofiler;

void ex_dbgui_init();

void ex_dbgui_begin_profiler();

void ex_dbgui_end_profiler();

void ex_dbgui_render_profiler();

#endif // EX_DBGUI_H