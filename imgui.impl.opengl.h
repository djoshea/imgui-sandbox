#ifndef __IMGUI_IMPL_OPENGL_INCLUDED__
#define __IMGUI_IMPL_OPENGL_INCLUDED__

namespace ImGuiImpl {
    void Init(); // call once at beggining
    bool ShouldClose(); // use in loop as while(!ImGuiImpl::GuiShouldClose()) { ... }
    void BeginFrame(); // call first in while loop
    void RenderFrame(); // call at end of ImGui calls
    void Shutdown(); // call once at end
}; // namespace ImGuiImpl

#endif
