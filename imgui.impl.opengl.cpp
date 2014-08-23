#include "imgui.impl.opengl.h"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"				// for .png loading
#include "imgui.h"
#ifdef _MSC_VER
#pragma warning (disable: 4996)		// 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#endif

#ifdef __APPLE__
#define nullptr NULL
#endif

static double time = 0;

static GLFWwindow* window;
static GLuint vbo;
static GLuint vao;
static GLuint vertexShader;
static GLuint fragmentShader;
static GLuint shaderProgram;
static GLuint fontTex;

static void InitGL();
static void InitImGui();

// This is the main rendering function that you have to implement and provide to ImGui (via setting up 'RenderDrawListsFn' in the ImGuiIO structuer)
// We are using the fixed pipeline.
// A faster way would be to collate all vertices from all cmd_lists into a single vertex buffer
static void ImImpl_RenderDrawLists(ImDrawList** const cmd_lists, int cmd_lists_count)
{
    if (cmd_lists_count == 0)
        return;

    // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, vertex/texcoord/color pointers.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    // Setup texture
    glBindTexture(GL_TEXTURE_2D, fontTex);
    glEnable(GL_TEXTURE_2D);

    // Setup orthographic projection matrix
    const float width = ImGui::GetIO().DisplaySize.x;
    const float height = ImGui::GetIO().DisplaySize.y;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0f, width, height, 0.0f, -1.0f, +1.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Render command lists
    for (int n = 0; n < cmd_lists_count; n++)
    {
        const ImDrawList* cmd_list = cmd_lists[n];
        const unsigned char* vtx_buffer = (const unsigned char*)cmd_list->vtx_buffer.begin();
        glVertexPointer(2, GL_FLOAT, sizeof(ImDrawVert), (void*)(vtx_buffer));
        glTexCoordPointer(2, GL_FLOAT, sizeof(ImDrawVert), (void*)(vtx_buffer+8));
        glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(ImDrawVert), (void*)(vtx_buffer+16));

        int vtx_offset = 0;
        const ImDrawCmd* pcmd_end = cmd_list->commands.end();
        for (const ImDrawCmd* pcmd = cmd_list->commands.begin(); pcmd != pcmd_end; pcmd++)
        {
            glScissor((int)pcmd->clip_rect.x, (int)(height - pcmd->clip_rect.w), (int)(pcmd->clip_rect.z - pcmd->clip_rect.x), (int)(pcmd->clip_rect.w - pcmd->clip_rect.y));
            glDrawArrays(GL_TRIANGLES, vtx_offset, pcmd->vtx_count);
            vtx_offset += pcmd->vtx_count;
        }
    }
    glDisable(GL_SCISSOR_TEST);
}

static const char* ImImpl_GetClipboardTextFn()
{
    return glfwGetClipboardString(window);
}

static void ImImpl_SetClipboardTextFn(const char* text, const char* text_end)
{
    if (!text_end)
        text_end = text + strlen(text);

    if (*text_end == 0)
    {
        // Already got a zero-terminator at 'text_end', we don't need to add one
        glfwSetClipboardString(window, text);
    }
    else
    {
        // Add a zero-terminator because glfw function doesn't take a size
        char* buf = (char*)malloc(text_end - text + 1);
        memcpy(buf, text, text_end-text);
        buf[text_end-text] = '\0';
        glfwSetClipboardString(window, buf);
        free(buf);
    }
}

// GLFW callbacks to get events
static void glfw_error_callback(int error, const char* description)
{
    fputs(description, stderr);
}

static void glfw_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    ImGuiIO& io = ImGui::GetIO();
    io.MouseWheel = (yoffset != 0.0f) ? yoffset > 0.0f ? 1 : - 1 : 0;           // Mouse wheel: -1,0,+1
}

static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    ImGuiIO& io = ImGui::GetIO();
    if (action == GLFW_PRESS)
        io.KeysDown[key] = true;
    if (action == GLFW_RELEASE)
        io.KeysDown[key] = false;
    io.KeyCtrl = (mods & GLFW_MOD_CONTROL) != 0;
    io.KeyShift = (mods & GLFW_MOD_SHIFT) != 0;
}

static void glfw_char_callback(GLFWwindow* window, unsigned int c)
{
    if (c > 0 && c <= 255)
        ImGui::GetIO().AddInputCharacter((char)c);
}

// OpenGL code based on http://open.gl tutorials
static void InitGL()
{
    glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit())
        exit(1);

    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    window = glfwCreateWindow(1280, 720, "ImGui OpenGL example", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, glfw_key_callback);
    glfwSetScrollCallback(window, glfw_scroll_callback);
    glfwSetCharCallback(window, glfw_char_callback);

    glewInit();
}

static void InitImGui()
{
    int w, h;
    glfwGetWindowSize(window, &w, &h);

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)w, (float)h);            // Display size, in pixels. For clamping windows positions.
    io.DeltaTime = 1.0f/60.0f;                                // Time elapsed since last frame, in seconds (in this sample app we'll override this every frame because our timestep is variable)
    io.PixelCenterOffset = 0.5f;                            // Align OpenGL texels
    io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;                    // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
    io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
    io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
    io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
    io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
    io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
    io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
    io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
    io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
    io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
    io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
    io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
    io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
    io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

    io.RenderDrawListsFn = ImImpl_RenderDrawLists;
    io.SetClipboardTextFn = ImImpl_SetClipboardTextFn;
    io.GetClipboardTextFn = ImImpl_GetClipboardTextFn;

    // Load font texture
    glGenTextures(1, &fontTex);
    glBindTexture(GL_TEXTURE_2D, fontTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    const void* png_data;
    unsigned int png_size;
    ImGui::GetDefaultFontData(NULL, NULL, &png_data, &png_size);
    int tex_x, tex_y, tex_comp;
    void* tex_data = stbi_load_from_memory((const unsigned char*)png_data, (int)png_size, &tex_x, &tex_y, &tex_comp, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_x, tex_y, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex_data);
    stbi_image_free(tex_data);
}

static void UpdateImGui()
{
    ImGuiIO& io = ImGui::GetIO();

    // Setup timestep
    static double time = 0.0f;
    const double current_time =  glfwGetTime();
    io.DeltaTime = (float)(current_time - time);
    time = current_time;

    // Setup inputs
    // (we already got mouse wheel, keyboard keys & characters from glfw callbacks polled in glfwPollEvents())
    double mouse_x, mouse_y;
    glfwGetCursorPos(window, &mouse_x, &mouse_y);
    io.MousePos = ImVec2((float)mouse_x, (float)mouse_y);                           // Mouse position, in pixels (set to -1,-1 if no mouse / on another screen, etc.)
    io.MouseDown[0] = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) != 0;
    io.MouseDown[1] = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) != 0;

    // Start the frame
    ImGui::NewFrame();
}

// client facing methods
namespace ImGuiImpl {
    void Init() {
        InitGL();
        InitImGui();
    }

    bool ShouldClose() {
        return glfwWindowShouldClose(window) != 0;
    }

    void BeginFrame() {
        ImGuiIO& io = ImGui::GetIO();
        io.MouseWheel = 0;
        glfwPollEvents();
        UpdateImGui();
    }

    void RenderFrame() {
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui::Render();

        glfwSwapBuffers(window);
    }

    void Shutdown()
    {
        ImGui::Shutdown();
        glfwTerminate();
    }

}; // namespace ImGuiImpl
