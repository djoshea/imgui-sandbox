#ifndef __IMGUI_CUSTOM_H_INCLUDED__
#define __IMGUI_CUSTOM_H_INCLUDED__

#include "imgui.h"

namespace ImGui {

    void PlotAnalog(const char* label, const float* values, int values_count, int values_offset = 0, const char* overlay_text = NULL, float scale_min = FLT_MAX, float scale_max = FLT_MAX, ImVec2 graph_size = ImVec2(0,0), size_t stride = sizeof(float));

}; // namespace ImGui

#endif
