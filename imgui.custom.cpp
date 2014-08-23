#include "imgui.cpp"

namespace ImGui {

void PlotAnalog(const char* label, const float* values, int values_count, int values_offset, const char* overlay_text, float scale_min, float scale_max, ImVec2 graph_size, size_t stride)
{
    ImGuiPlotType plot_type = ImGuiPlotType_Lines;
    ImGuiState& g = GImGui;
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;

    const ImGuiStyle& style = g.Style;

    const ImVec2 text_size = CalcTextSize(label);
    if (graph_size.x == 0)
        graph_size.x = window->DC.ItemWidth.back();
    if (graph_size.y == 0)
        graph_size.y = text_size.y;

    const ImGuiAabb frame_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(graph_size.x, graph_size.y) + style.FramePadding*2.0f);
    const ImGuiAabb graph_bb(frame_bb.Min + style.FramePadding, frame_bb.Max - style.FramePadding);
    const ImGuiAabb bb(frame_bb.Min, frame_bb.Max + ImVec2(style.ItemInnerSpacing.x + text_size.x,0));
    ItemSize(bb);

    if (ClipAdvance(bb))
        return;

    // Determine scale if not specified
    if (scale_min == FLT_MAX || scale_max == FLT_MAX)
    {
        float v_min = FLT_MAX;
        float v_max = -FLT_MAX;
        for (int i = 0; i < values_count; i++)
        {
            const float v = PlotGetValue(values, stride, i);
            v_min = ImMin(v_min, v);
            v_max = ImMax(v_max, v);
        }
        if (scale_min == FLT_MAX)
            scale_min = v_min;
        if (scale_max == FLT_MAX)
            scale_max = v_max;
    }

    RenderFrame(frame_bb.Min, frame_bb.Max, window->Color(ImGuiCol_FrameBg));

    int res_w = ImMin((int)graph_size.x, values_count);
    if (plot_type == ImGuiPlotType_Lines)
        res_w -= 1;

    // Tooltip on hover
    int v_hovered = -1;
    if (IsMouseHoveringBox(graph_bb))
    {
        const float t = ImClamp((g.IO.MousePos.x - graph_bb.Min.x) / (graph_bb.Max.x - graph_bb.Min.x), 0.0f, 0.9999f);
        const int v_idx = (int)(t * (values_count + ((plot_type == ImGuiPlotType_Lines) ? -1 : 0)));
        IM_ASSERT(v_idx >= 0 && v_idx < values_count);

        const float v0 = PlotGetValue(values, stride, (v_idx + values_offset) % values_count);
        const float v1 = PlotGetValue(values, stride, (v_idx + 1 + values_offset) % values_count);
        if (plot_type == ImGuiPlotType_Lines)
            ImGui::SetTooltip("%d: %8.4g\n%d: %8.4g", v_idx, v0, v_idx+1, v1);
        else if (plot_type == ImGuiPlotType_Histogram)
            ImGui::SetTooltip("%d: %8.4g", v_idx, v0);
        v_hovered = v_idx;
    }

    const float t_step = 1.0f / (float)res_w;

    float v0 = PlotGetValue(values, stride, (0 + values_offset) % values_count);
    float t0 = 0.0f;
    ImVec2 p0 = ImVec2( t0, 1.0f - ImSaturate((v0 - scale_min) / (scale_max - scale_min)) );

    const ImU32 col_base = window->Color((plot_type == ImGuiPlotType_Lines) ? ImGuiCol_PlotLines : ImGuiCol_PlotHistogram);
    const ImU32 col_hovered = window->Color((plot_type == ImGuiPlotType_Lines) ? ImGuiCol_PlotLinesHovered : ImGuiCol_PlotHistogramHovered);

    for (int n = 0; n < res_w; n++)
    {
        const float t1 = t0 + t_step;
        const int v_idx = (int)(t0 * values_count);
        IM_ASSERT(v_idx >= 0 && v_idx < values_count);
        const float v1 = PlotGetValue(values, stride, (v_idx + values_offset + 1) % values_count);
        const ImVec2 p1 = ImVec2( t1, 1.0f - ImSaturate((v1 - scale_min) / (scale_max - scale_min)) );

        // NB: draw calls are merged into ones
        if (plot_type == ImGuiPlotType_Lines)
            window->DrawList->AddLine(ImLerp(graph_bb.Min, graph_bb.Max, p0), ImLerp(graph_bb.Min, graph_bb.Max, p1), v_hovered == v_idx ? col_hovered : col_base);
        else if (plot_type == ImGuiPlotType_Histogram)
            window->DrawList->AddRectFilled(ImLerp(graph_bb.Min, graph_bb.Max, p0), ImLerp(graph_bb.Min, graph_bb.Max, ImVec2(p1.x, 1.0f))+ImVec2(-1,0), v_hovered == v_idx ? col_hovered : col_base);

        v0 = v1;
        t0 = t1;
        p0 = p1;
    }

    // Overlay last value
    if (overlay_text)
        RenderText(ImVec2(graph_bb.GetCenter().x-CalcTextSize(overlay_text).x*0.5f, frame_bb.Min.y + style.FramePadding.y), overlay_text);

    RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, graph_bb.Min.y), label);
}

};

