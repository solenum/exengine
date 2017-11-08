#include "window.h"
#include "octree.h"
#include "dbgui.h"

ex_scene_t *scene = NULL;

const int ex_dbgprofiler_width  = 640;
const int ex_dbgprofiler_height = 250;

ex_dbgprofiler_t ex_dbgprofiler;
struct ImVec4 ex_profiler_colors[] = {
  {0.318f, 0.839f, 1.0f, 1.0f},
  {0.322f, 1.0f, 0.827f, 1.0f},
  {0.91f, 0.341f, 0.647f, 1.0f},
  {1.0f, 0.827f, 0.322f, 1.0f},
  {0.9f, 0.627f, 0.222f, 1.0f},
  {0.6f, 0.3f, 0.4f, 1.0f},
};

float scale_to_range(float input, float mina, float maxa, float minb, float maxb) {
  return (maxa - mina) * (input - minb) / (maxb - minb) + mina;
}

void ex_dbgui_init(ex_scene_t *s)
{
  scene = s;

  // set default starting values
  for (int i=0; i<ex_dbgprofiler_count; i++) {
    ex_dbgprofiler.begin[i] = 0.0f;
    ex_dbgprofiler.end[i]   = 1.0f;
  }

  for (int i=0; i<64; i++)
    ex_dbgprofiler.ex_frame_times[i] = 0.0f;

  ex_dbgprofiler.timer           = 0.0f;
  ex_dbgprofiler.wireframe       = 0;
  ex_dbgprofiler.last_ex_frame_time = 0;
  ex_dbgprofiler.render_octree   = 0;
  ex_dbgprofiler.octree_obj_only = 0;

  // set gui style
  igPushStyleVarVec(ImGuiStyleVar_FramePadding, (struct ImVec2){0.0f, 0.0f});
  igPushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
}

void ex_dbgui_begin_profiler()
{
  if (!ex_dbgprofiler.paused)
    ex_dbgprofiler.delta_begin = glfwGetTime();
}

void ex_dbgui_end_profiler()
{
  if (!ex_dbgprofiler.paused) {
    ex_dbgprofiler.delta_end = glfwGetTime();
    ex_dbgprofiler.timer += (ex_dbgprofiler.delta_end - ex_dbgprofiler.delta_begin);
    
    if (ex_dbgprofiler.timer > 0.1f) {
      ex_dbgprofiler.timer = 0.0f;
      ex_dbgprofiler.delta_time = (ex_dbgprofiler.delta_end - ex_dbgprofiler.delta_begin);
      ex_dbgprofiler.ex_frame_times[ex_dbgprofiler.last_ex_frame_time++] = (int)(1.0f/ex_dbgprofiler.delta_time);
      for (int i=0; i<ex_dbgprofiler_count; i++) {
        ex_dbgprofiler.values[i] = (ex_dbgprofiler.end[i] - ex_dbgprofiler.begin[i]);
      }
    
      ex_dbgprofiler.values[ex_dbgprofiler_other] -= ex_dbgprofiler.values[ex_dbgprofiler_update];
    }

    if (ex_dbgprofiler.last_ex_frame_time >= 128)
      ex_dbgprofiler.last_ex_frame_time = 0;
  }
}

void ex_dbgui_render_profiler()
{
  float ex_frame_time = ex_dbgprofiler.delta_time;
  double val;
 
  igSetNextWindowSize((struct ImVec2){ex_dbgprofiler_width, ex_dbgprofiler_height}, 0);
  igBegin("Render Profiler", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);

  igCheckbox("Pause ", (bool*)&ex_dbgprofiler.paused);
  igSameLine(0.0f, 0.0f);
  igCheckbox("Wireframe ", (bool*)&ex_dbgprofiler.wireframe);
  igCheckbox("Octree ", (bool*)&ex_dbgprofiler.render_octree);
  igSameLine(0.0f, 0.0f);
  igCheckbox("Octree OBJ only ", (bool*)&ex_dbgprofiler.octree_obj_only);
  igSameLine(0.0f, 0.0f);
  igPushItemWidth(150.0f);
  igSliderInt("", &ex_octree_min_size, 1, 100, NULL);
  igSameLine(0.0f, 8.0f);
  int build = igButton("Rebuild Tree", (struct ImVec2){128.0f, 13.0f});
  if (build)
    scene->collision_built = 0;
  igText("Render Time %i FPS (%.2fms)", (int)(1.0/ex_frame_time), 1000.0/(1.0/ex_frame_time));
  igNewLine();

  float last_offset = 0.0f;
  for (int i=0; i<ex_dbgprofiler_count; i++) {
    float x = ex_dbgprofiler.values[i];
    val = 1000.0/(1.0/x);
    x = scale_to_range(x, 24.0f, ex_dbgprofiler_width, 0.0f, ex_frame_time);
    
    if (i == ex_dbgprofiler_count-1)
      x = ex_dbgprofiler_width;

    igSameLine(last_offset, 0.0f);
    igPushStyleColor(ImGuiCol_Button, ex_profiler_colors[i]);
    igButton("", (struct ImVec2){x, 8.0f});
    if (igIsItemHovered())
      igSetTooltip("%s (%2.fms)", ex_dbgprofiler_strings_full[i], val);
    igPopStyleColor(1);

    last_offset += x;
  }

  last_offset = 0.0f;
  for (int i=0; i<ex_dbgprofiler_count; i++) {
    float x = ex_dbgprofiler.values[i];
    val = 1000.0/(1.0/x);
    x = scale_to_range(x, 8.0f, ex_dbgprofiler_width*0.5f, 0.0f, ex_frame_time);

    igPushStyleColor(ImGuiCol_Button, ex_profiler_colors[i]);
    igButton("", (struct ImVec2){x, 16.0f});
    if (igIsItemHovered())
      igSetTooltip("%s (%.4fms)", ex_dbgprofiler_strings_full[i], val);
    igSameLine(0.0f, 0.0f);
    igTextColored(ex_profiler_colors[i], "  %s (%.4fms)", ex_dbgprofiler_strings_full[i], val);
    igPopStyleColor(1);

    last_offset += x;
  }

  igPushStyleColor(ImGuiCol_FrameBg, (struct ImVec4){0.0f, 0.0f, 0.0f, 0.0f});
  igPlotLines("", ex_dbgprofiler.ex_frame_times, 128, 0, NULL, 0.0f, 2000.0f, (struct ImVec2){ex_dbgprofiler_width, 64.0f}, sizeof(float));
  igPopStyleColor(1);
  igEnd();
}