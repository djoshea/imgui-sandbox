#include "imgui.custom.h"
#include "imgui.impl.opengl.h"
#include <string.h> // memset
#include <math.h> // sqrt
#include <stdio.h> // vsnprintf

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

// Convert hsv floats ([0-1],[0-1],[0-1]) to rgb floats ([0-1],[0-1],[0-1]), from Foley & van Dam p593
// also http://en.wikipedia.org/wiki/HSL_and_HSV
static void ImConvertColorHSVtoRGB(float h, float s, float v, float& out_r, float& out_g, float& out_b)
{
    if (s == 0.0f)
    {
        // gray
        out_r = out_g = out_b = v;
        return;
    }

    h = fmodf(h, 1.0f) / (60.0f/360.0f);
    int   i = (int)h;
    float f = h - (float)i;
    float p = v * (1.0f - s);
    float q = v * (1.0f - s * f);
    float t = v * (1.0f - s * (1.0f - f));

    switch (i)
    {
    case 0: out_r = v; out_g = t; out_b = p; break;
    case 1: out_r = q; out_g = v; out_b = p; break;
    case 2: out_r = p; out_g = v; out_b = t; break;
    case 3: out_r = p; out_g = q; out_b = v; break;
    case 4: out_r = t; out_g = p; out_b = v; break;
    case 5: default: out_r = v; out_g = p; out_b = q; break;
    }
}

enum LogEntryType {
    Log_Message,
    Log_Warning,
    Log_Error,
    Log_Debug,
    Log_GuiDebug,
};

struct Log {
    ImGuiTextBuffer buffer;

public:
    void clear()
    {
        buffer.clear();
    }

    void append(LogEntryType type, const char* fmt, ...)
    {
        const int buf_size = 1000;
        char buf[buf_size];
        va_list args;
        va_start(args, fmt);
        int w = vsnprintf(buf, buf_size, fmt, args);
        va_end(args);
        buf[buf_size-1] = 0;

        char typeStr[20];
        switch(type) {
            case Log_Message:
                strcpy(typeStr, "Message");
            case Log_Warning:
                strcpy(typeStr, "Warning");
            case Log_Error:
                strcpy(typeStr, "Error");
            case Log_Debug:
                strcpy(typeStr, "Debug");
            case Log_GuiDebug:
                strcpy(typeStr, "Gui Debug");
        }
        buffer.append("%s: %s\n", typeStr, buf);
    }

    void show()
    {
        if (ImGui::Button("Clear")) { clear(); }
        ImGui::TextUnformatted(buffer.begin(), buffer.end());
    }
};

int main(int argc, char** argv)
{
    ImGuiImpl::Init();

    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.40f, 0.40f, 0.40f, 0.30f);   // Background of checkbox, radio button, plot, slider, text input

    static Log mainLog;
    mainLog.clear();
    while (!ImGuiImpl::ShouldClose())
    {
        ImGuiImpl::BeginFrame();

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

        if (ImGui::CollapsingHeader("Log"))
        {
            if (ImGui::Button("Add to Log"))
            {
                mainLog.append(Log_Message, "Hello world!\n");
            }
            mainLog.show();
        }

        if (show_graph)
        {
            GraphWindow(&show_graph);
        }

        bool show_test;
        ImGui::ShowTestWindow(&show_test);

        ImGuiImpl::RenderFrame();
    }

    ImGuiImpl::Shutdown();
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
    ImGui::Begin("Graph", open, ImVec2(500,680), fill_alpha, layout_flags);
    ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.80f);

    const int num_graphs = 6;

    static ImVector<float> values[num_graphs];
    static int values_offset = 0;

    static float refresh_time = -1.0f;
    static float phase[num_graphs];
    bool refresh = false;

    const float frame_time = 1.0f/60.0f;
    float time = ImGui::GetTime();
   // if (time > refresh_time + frame_time)
    //{
        time = ImGui::GetTime();
        refresh = true;
        refresh_time = time;
    //}

    // void PlotLines(const char* label, const float* values, int values_count, int values_offset, const char* overlay_text, float scale_min, float scale_max, ImVec2 graph_size)
    double amp_freq = 0.1;

    double freq = 3;
    for(int g = 0; g < num_graphs; g++)
    {
        if (values[g].empty()) { values[g].resize(100); memset(&values[g].front(), 0, values[g].size()*sizeof(float)); }

        if(refresh)
        {
            // create dummy data at 60 hz
            values[g][values_offset] = 2.0f*sin(2.0f*M_PI*amp_freq*time)*sin(2.0f*M_PI*freq*time);
        }

        ImVec4 colRGB = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        double hue_norm = (double)g / (double)num_graphs;
        ImConvertColorHSVtoRGB(hue_norm, .5f, 1.0f, colRGB.x, colRGB.y, colRGB.z);

        char label[30];
        snprintf(label, 30, "%.2f Hz", freq);
        ImGui::PlotAnalog(label, "mV", &values[g].front(), values[g].size() - 1, (values_offset+1)%values[g].size(),
            "", 1.0f, 1.0f, true, ImVec2(0, 80), sizeof(float), colRGB);
        freq *= 1.25f;
    }
    if(refresh)
        values_offset = (values_offset+1)%values[0].size();

    ImGui::End();
}

