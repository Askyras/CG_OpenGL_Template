#include "CG/application.h"

#include "GL/glew.h"
#include "SDL3/SDL.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"

#include <exception>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

using namespace CG;


static void GLAPIENTRY gl_message_callback(
    GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const void* userParam)
{
    if (type != GL_DEBUG_TYPE_ERROR) return;
    // Set a debug breakpoint here to quickly find the source of the error (will be the previous OpenGL call)
    printf("[OpenGL] Error 0x%04X: %s\n", type, message);
}


CG::Application::Application(const char* title, int width, int height)
    : viewport(width, height)
{
	init(title, width, height);
}


void CG::Application::init(const char* win_title, int win_width, int win_height)
{
    if (initialized) return;

    std::set_terminate([]() {
        try {
            std::exception_ptr eptr{ std::current_exception() };
            if (eptr) {
                std::rethrow_exception(eptr);
            }
            else {
                printf("Exiting without exception\n");
            }
        }
        catch (const std::exception& e) {
            printf("Uncaught exception: %s\n", e.what());
        }
        exit(EXIT_FAILURE);
        });

    // Setup SDL
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
    {
        printf("Error: SDL_Init(): %s\n", SDL_GetError());
        std::terminate();
    }

    // GL 4.1 + GLSL 400
    const char* glsl_version = "#version 400";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    SDL_WindowFlags window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    window = SDL_CreateWindow(win_title, (int)(win_width * main_scale), (int)(win_height * main_scale), window_flags);
    if (window == nullptr)
    {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        std::terminate();
    }
    gl_context = SDL_GL_CreateContext(window);
    if (gl_context == nullptr)
    {
        printf("Error: SDL_GL_CreateContext(): %s\n", SDL_GetError());
        std::terminate();
    }

    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(window);
    SDL_SetHint(SDL_HINT_MOUSE_RELATIVE_MODE_CENTER, "1");

    // Setup GLEW
    glewExperimental = 1;
    if (glewInit() != GLEW_OK) {
        printf("Error: Failed to setup GLEW\n");
        std::terminate();
    }

    const GLubyte* versionstr = glGetString(GL_VERSION);
    const GLubyte* vendorstr = glGetString(GL_VENDOR);
    if (versionstr && vendorstr) {
        printf("Running OpenGL %s on %s hardware\n", versionstr, vendorstr);
    }
    else {
        printf("Failed to get OpenGL version or vendor\n");
    }

    // OpenGL logging
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(gl_message_callback, nullptr);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.IniFilename = NULL;									  // Disable saving layout to ini file
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;   // Disable so that it doesn't interfere with our own cursor management

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale;        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If fonts are not explicitly loaded, Dear ImGui will select an embedded font: either AddFontDefaultVector() or AddFontDefaultBitmap().
    //   This selection is based on (style.FontSizeBase * style.FontScaleMain * style.FontScaleDpi) reaching a small threshold.
    // - You can load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - If a file cannot be loaded, AddFont functions will return a nullptr. Please handle those errors in your code (e.g. use an assertion, display an error and quit).
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use FreeType for higher quality font rendering.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.
    //style.FontSizeBase = 20.0f;
    io.Fonts->AddFontDefaultVector();
    //io.Fonts->AddFontDefaultBitmap();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf");
    //io.Fonts->AddFontFromFileTTF("fonts/DroidSans.ttf");
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf");
    //IM_ASSERT(font != nullptr);
	
    initialized = true;
}


CG::Application::~Application()
{
    // Cleanup
    // [If using SDL_MAIN_USE_CALLBACKS: all code below would likely be your SDL_AppQuit() function]
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DestroyContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}


bool CG::Application::new_frame()
{
    // Poll and handle events (inputs, window resize, etc.)
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
    // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
    // [If using SDL_MAIN_USE_CALLBACKS: call ImGui_ImplSDL3_ProcessEvent() from your SDL_AppEvent() function]
    input.clear();
    SDL_GetMouseState(&input.mouse_pos.x, &input.mouse_pos.y);

    SDL_Event evt;
    while (SDL_PollEvent(&evt))
    {
        switch (evt.type)
        {
        case SDL_EVENT_QUIT: {
            quit = true;
        } break;
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED: {
            if (evt.window.windowID == SDL_GetWindowID(window))
                quit = true;
        } break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN: {
            input.mouse_clicked |= SDL_BUTTON_MASK(evt.button.button);
        } break;
        case SDL_EVENT_MOUSE_WHEEL: {
            input.mouse_scroll = evt.wheel.integer_y;
        } break;
        case SDL_EVENT_KEY_DOWN: {
            input.key_pressed[evt.key.scancode] = true;
        } break;
        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: {
            viewport.x = evt.window.data1;
            viewport.y = evt.window.data2;
        } break;
        default: break;
        }
        if (!(mouse_locked && evt.type == SDL_EVENT_MOUSE_MOTION)) {
            ImGui_ImplSDL3_ProcessEvent(&evt);
        }
        handle_event(&evt);
    }
    // Unfortunately this function is quite fragile, as calling it again resets the reference position, so don't use it anywhere!
	SDL_GetRelativeMouseState(&input.mouse_delta.x, &input.mouse_delta.y);

    // [If using SDL_MAIN_USE_CALLBACKS: all code below would likely be your SDL_AppIterate() function]
    if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED)
    {
        SDL_Delay(10);
        return false;
    }

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    return !quit;
}

void CG::Application::lock_mouse()
{
    if (mouse_locked)
        return;
    mouse_locked = true;
    glm::vec2 mouse_pos;
    SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);
    this->original_mouse_pos = mouse_pos; // to restore afterwards
    SDL_SetWindowRelativeMouseMode(window, true);
}

void CG::Application::unlock_mouse()
{
    if (!mouse_locked)
        return;
    mouse_locked = false;
    SDL_WarpMouseInWindow(window, this->original_mouse_pos.x, this->original_mouse_pos.y);
    SDL_SetWindowRelativeMouseMode(window, false);
}


void CG::run(Application& app)
{
    if (!app.initialized)
        throw std::logic_error("Application was not initialized before calling run()");

    app.last_frame_time_ns = SDL_GetTicksNS();

    while (!app.quit)
    {
        auto current_time = SDL_GetTicksNS();
        double dt = (current_time - app.last_frame_time_ns) * 1e-9;
        app.last_frame_time_ns = current_time;

        if (!app.new_frame())
            continue;
        app.update(dt);
        app.render();

        ImGui::Render();
        ImGuiIO& io = ImGui::GetIO();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(app.window);
    }
}