#ifndef __IMGUI_CUSTOM_H_INCLUDED__
#define __IMGUI_CUSTOM_H_INCLUDED__

#include "imgui.h"

namespace ImGui {
    void PlotAnalog(const char* label, const char* units, const float* values, int values_count, int values_offset = 0,
        const char* overlay_text = NULL, float scale_min = FLT_MAX, float scale_max = FLT_MAX, bool auto_expand_scale = true,
        ImVec2 graph_size = ImVec2(0,0), size_t stride = sizeof(float), ImVec4 line_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
}; // namespace ImGui

#endif
