#include "imgui.h"
#include "gui.h"
#include <string.h>		// memset
#include <math.h>		// sqrt

#include <stdio.h>		// vsnprintf
void GraphWindow(bool* open);

#undef ARRAYSIZE
#define ARRAYSIZE(_ARR)			(sizeof(_ARR)/sizeof(*_ARR))

static size_t ImFormatString(char* buf, size_t buf_size, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int w = vsnprintf(buf, buf_size, fmt, args);
	va_end(args);
	buf[buf_size-1] = 0;
	if (w == -1) w = buf_size;
	return w;
}

int main(int argc, char** argv)
{
    InitGL();
    InitImGui();

    while (!GuiShouldClose())
    {
        BeginFrame();

        // 2) ImGui usage
        static bool show_graph = true;
        static float f;
        ImGui::Text("Hello, world!");
        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
        show_graph ^= ImGui::Button("Show Graphs");

        // Calculate and show framerate
        static float ms_per_frame[120] = { 0 };
        static int ms_per_frame_idx = 0;
        static float ms_per_frame_accum = 0.0f;
        ms_per_frame_accum -= ms_per_frame[ms_per_frame_idx];

        ImGuiIO& io = ImGui::GetIO();
        ms_per_frame[ms_per_frame_idx] = io.DeltaTime * 1000.0f;
        ms_per_frame_accum += ms_per_frame[ms_per_frame_idx];
        ms_per_frame_idx = (ms_per_frame_idx + 1) % 120;
        const float ms_per_frame_avg = ms_per_frame_accum / 120;
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", ms_per_frame_avg, 1000.0f / ms_per_frame_avg);

        if (show_graph)
        {
            GraphWindow(&show_graph);
        }

        RenderFrame();
    }

    Shutdown();
    return 0;
}

void GraphWindow(bool* open)
{
    static bool no_titlebar = false;
    static bool no_border = true;
    static bool no_resize = false;
    static bool no_move = false;
    static bool no_scrollbar = false;
    static float fill_alpha = 0.65f;

    const ImU32 layout_flags = (no_titlebar ? ImGuiWindowFlags_NoTitleBar : 0) | (no_border ? 0 : ImGuiWindowFlags_ShowBorders) | (no_resize ? ImGuiWindowFlags_NoResize : 0) | (no_move ? ImGuiWindowFlags_NoMove : 0) | (no_scrollbar ? ImGuiWindowFlags_NoScrollbar : 0);
    ImGui::Begin("Graph", open, ImVec2(550,680), fill_alpha, layout_flags);
    //ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.65f);

    static int num_graphs = 1;
    ImGui::SliderInt("Num Graphs", &num_graphs, 0, 100);
    ImGui::Spacing();

    if (ImGui::CollapsingHeader("Graphs widgets", NULL, false, true))
    {
        static bool pause;
        static ImVector<float> values;
        if (values.empty()) { values.resize(100); memset(&values.front(), 0, values.size()*sizeof(float)); }
        static int values_offset = 0;
        if (!pause)
        {
            // create dummy data at 60 hz
            static float refresh_time = -1.0f;
            if (ImGui::GetTime() > refresh_time + 1.0f/60.0f)
            {
                refresh_time = ImGui::GetTime();
                static float phase = 0.0f;
                values[values_offset] = cos(phase);
                values_offset = (values_offset+1)%values.size();
                phase += 0.10f*values_offset;
            }
        }
        // void PlotLines(const char* label, const float* values, int values_count, int values_offset, const char* overlay_text, float scale_min, float scale_max, ImVec2 graph_size)
        for(int g = 0; g < num_graphs; g++) {
            ImGui::PlotTight("Values", &values.front(), values.size(), (values_offset+3*g)%values.size(),
                    "", -2.0f, 2.0f, ImVec2(100,200));
        }
    }

    ImGui::End();
}

