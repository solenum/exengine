#include "window.h"
#include "dbgui.h"

const int ex_dbgprofiler_width  = 640;
const int ex_dbgprofiler_height = 250;

ex_dbgprofiler_t ex_dbgprofiler;
struct ImVec4 ex_profiler_colors[] = {
  {0.318f, 0.839f, 1.0f, 1.0f},
  {0.322f, 1.0f, 0.827f, 1.0f},
  {0.91f, 0.341f, 0.647f, 1.0f},
  {1.0f, 0.827f, 0.322f, 1.0f},
  {0.6f, 0.3f, 0.4f, 1.0f},
};

float scale_to_range(float input, float mina, float maxa, float minb, float maxb) {
  return (maxa - mina) * (input - minb) / (maxb - minb) + mina;
}

void ex_dbgui_init()
{
  // set default starting values
  for (int i=0; i<ex_dbgprofiler_count; i++) {
    ex_dbgprofiler.begin[i] = 0.0f;
    ex_dbgprofiler.end[i]   = 1.0f;
  }

  for (int i=0; i<64; i++)
    ex_dbgprofiler.frame_times[i] = 0.0f;

  ex_dbgprofiler.timer = 0.0f;
  ex_dbgprofiler.last_frame_time = 0;

  // set gui style
  igPushStyleVarVec(ImGuiStyleVar_FramePadding, (struct ImVec2){0.0f, 0.0f});
  igPushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
}

void ex_dbgui_begin_profiler()
{
  if (!ex_dbgprofiler.paused)
    ex_dbgprofiler.delta_begin = (float)glfwGetTime();
}

void ex_dbgui_end_profiler()
{
  if (!ex_dbgprofiler.paused) {
    ex_dbgprofiler.delta_end = (float)glfwGetTime();
    ex_dbgprofiler.timer += (ex_dbgprofiler.delta_end - ex_dbgprofiler.delta_begin);
    
    if (ex_dbgprofiler.timer > 0.1f) {
      ex_dbgprofiler.timer = 0.0f;
      ex_dbgprofiler.delta_time = (ex_dbgprofiler.delta_end - ex_dbgprofiler.delta_begin);
      ex_dbgprofiler.frame_times[ex_dbgprofiler.last_frame_time++] = (int)(1.0f/ex_dbgprofiler.delta_time);
      for (int i=0; i<ex_dbgprofiler_count; i++) {
        ex_dbgprofiler.values[i] = ex_dbgprofiler.end[i] - ex_dbgprofiler.begin[i];
      }
    }

    if (ex_dbgprofiler.last_frame_time >= 128)
      ex_dbgprofiler.last_frame_time = 0;
  }
}

void ex_dbgui_render_profiler()
{
  float frame_time = ex_dbgprofiler.delta_time;
 
  igSetNextWindowSize((struct ImVec2){ex_dbgprofiler_width, ex_dbgprofiler_height}, 0);
  igBegin("Render Profiler", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);

  igCheckbox("Pause ", (bool*)&ex_dbgprofiler.paused);
  igText("Render Time %i FPS (%fms)", (int)(1.0f/frame_time), ex_dbgprofiler.delta_time);
  igNewLine();

  float last_offset = 0.0f;
  for (int i=0; i<ex_dbgprofiler_count; i++) {
    float x = ex_dbgprofiler.values[i];
    x = scale_to_range(x, 24.0f, 600.0f, 0.0f, frame_time);
    
    if (i == ex_dbgprofiler_count-1)
      x = ex_dbgprofiler_width;

    igSameLine(last_offset, 0.0f);
    igPushStyleColor(ImGuiCol_Button, ex_profiler_colors[i]);
    igButton("", (struct ImVec2){x, 8.0f});
    if (igIsItemHovered())
      igSetTooltip("%s (%fms)", ex_dbgprofiler_strings_full[i], ex_dbgprofiler.values[i]);
    igPopStyleColor(1);

    last_offset += x;
  }

  last_offset = 0.0f;
  for (int i=0; i<ex_dbgprofiler_count; i++) {
    float x = ex_dbgprofiler.values[i];
    x = scale_to_range(x, 8.0f, ex_dbgprofiler_width*0.5f, 0.0f, frame_time);

    igPushStyleColor(ImGuiCol_Button, ex_profiler_colors[i]);
    igButton("", (struct ImVec2){x, 16.0f});
    if (igIsItemHovered())
      igSetTooltip("%s (%fms)", ex_dbgprofiler_strings_full[i], ex_dbgprofiler.values[i]);
    igSameLine(0.0f, 0.0f);
    igTextColored(ex_profiler_colors[i], "  %s (%fms)", ex_dbgprofiler_strings_full[i], ex_dbgprofiler.values[i]);
    igPopStyleColor(1);

    last_offset += x;
  }

  igPushStyleColor(ImGuiCol_FrameBg, (struct ImVec4){0.0f, 0.0f, 0.0f, 0.0f});
  igPlotLines("", ex_dbgprofiler.frame_times, 128, 0, NULL, 0.0f, 250.0f, (struct ImVec2){ex_dbgprofiler_width, 64.0f}, sizeof(float));
  igPopStyleColor(1);
  igEnd();
}