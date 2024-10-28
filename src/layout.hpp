#pragma once

// Dear ImGui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"
#include "test_mocking_server.hpp"
#include <stdio.h>
#include <thread>
#include <chrono>
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

// This example can also compile and run with Emscripten! See 'Makefile.emscripten' for details.
#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

ImVec4 ColorFromRGB(int r, int g, int b, float a) {
    return ImVec4(r / 255.0f, g / 255.0f, b / 255.0f, a);
}



int heightOfNavbar = 50;
int widthOfSidebar = 50;
int heightOfFooter = 200;

float PlotTable_x1 = 0.0f;
float PlotTable_y1() {return float(heightOfNavbar); }
float PlotTable_x2 = 200.f;
float PlotTable_y2() {return ImGui::GetMainViewport()->Size.y - float(heightOfFooter); }


bool toggleChannelSelector = false;
DataManager data_manager;
std::thread ws_thread(test_mocking_server, std::ref(data_manager));

void ShowNavbar()
{
        
    float buttonRounding = 0.0f;
    ImVec4 navbarBg = ColorFromRGB(43, 43, 43, 1.0f);
    ImGui::Begin("Navbar", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);

    ImVec4 buttonColor = ColorFromRGB(255, 255, 255, 1.0f);
    ImVec4 buttonHoveredColor = ColorFromRGB(255, 255, 255, 0.6f);
    ImVec4 buttonActiveColor = ColorFromRGB(255, 255, 255, 1.0f);
    ImVec2 buttonSize = ImVec2(30,30);

    // Ensure the window is positioned at the top and spans the full width
    ImGui::SetWindowPos(ImVec2(0, 0));
    ImGui::SetWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, heightOfNavbar));

    ImGui::PushStyleColor(ImGuiCol_WindowBg, navbarBg); // Set the background color
    ImGui::PopStyleColor(1);


    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, buttonRounding);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(25, 25));

    ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, buttonHoveredColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, buttonActiveColor);

    if (ImGui::Button("Home", buttonSize))
    {
        // Corrected the assignment to comparison
        if (toggleChannelSelector == false) {
            toggleChannelSelector = true;
        } else {
            toggleChannelSelector = false;
        }
    }

    ImGui::SameLine();
    ImGui::PopStyleColor(3);

    // Similarly for other buttons
    ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, buttonHoveredColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, buttonActiveColor);
    if (ImGui::Button("Settings", buttonSize))
    {
        ImGui::Text("Adjust your settings here.");
    }
    ImGui::SameLine();
    ImGui::PopStyleColor(3);

    ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, buttonHoveredColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, buttonActiveColor);
    if (ImGui::Button("About", buttonSize))
    {
        ImGui::Text("About this application.");
    }
    ImGui::PopStyleColor(3);

    ImGui::PopStyleVar(2);
    ImGui::End();
}

void ShowSidebar()
{
  
    // Begin the sidebar window without title, move, nav, resize, or scrollbar
    ImGui::Begin("Sidebar", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);
    
    float buttonRounding = 0.0f;

    ImVec4 sidebarBg = ColorFromRGB(43, 43, 43, 1.0f);
    ImVec4 buttonColor = ColorFromRGB(255, 255, 255, 1.0f);
    ImVec4 buttonHoveredColor = ColorFromRGB(255, 255, 255, 0.6f);
    ImVec4 buttonActiveColor = ColorFromRGB(255, 255, 255, 1.0f);
    ImVec2 buttonSize = ImVec2(30, 30); // Set a wider button size for the sidebar

    // Position the sidebar on the right side and set the height accordingly
    ImGui::SetWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - widthOfSidebar, heightOfNavbar)); // 
    ImGui::SetWindowSize(ImVec2(widthOfSidebar, ImGui::GetIO().DisplaySize.y)); // 

    ImGui::PushStyleColor(ImGuiCol_WindowBg, sidebarBg); // Set the background color
    ImGui::PopStyleColor(1);

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, buttonRounding);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(25, 25));

    // Button for Home
    ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, buttonHoveredColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, buttonActiveColor);
    if (ImGui::Button("Home", buttonSize))
    {
        ImGui::Text("Welcome to the Home tab!");
    }
    ImGui::PopStyleColor(3);

    // Button for Settings
    ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, buttonHoveredColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, buttonActiveColor);
    if (ImGui::Button("Settings", buttonSize))
    {
        ImGui::Text("Adjust your settings here.");
    }
    ImGui::PopStyleColor(3);

    // Button for About
    ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, buttonHoveredColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, buttonActiveColor);
    if (ImGui::Button("About", buttonSize))
    {
        ImGui::Text("About this application.");
    }
    ImGui::PopStyleColor(3);

    ImGui::PopStyleVar(2);
    ImGui::End();
}

void ShowMainWindow() {


    ImVec2 window_size = ImVec2(ImGui::GetIO().DisplaySize.x - widthOfSidebar, ImGui::GetIO().DisplaySize.y - heightOfNavbar - heightOfFooter);
    ImVec2 window_position = ImVec2(0, heightOfNavbar);

    // Set the window position and size before beginning the window
    ImGui::SetNextWindowPos(window_position);
    ImGui::SetNextWindowSize(window_size);

    ImVec4 mainWindowBg = ImVec4(0.0f, 0.0f, 0.0f, 1.0f); 
    ImGui::PushStyleColor(ImGuiCol_WindowBg, mainWindowBg);

    // Begin the window
    if (ImGui::Begin("MainWindow", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBringToFrontOnFocus))
    {
        // Set the background color using RGBA directly, converting RGB 200, 0, 0 to ImGui's color format
        ImVec4 mainWindowBg = ImVec4(0.0f, 0.0f, 0.0f, 1.0f); // Red color
        ImGui::PushStyleColor(ImGuiCol_WindowBg, mainWindowBg); // Apply background color

        if (toggleChannelSelector) {
            data_manager.PlotTable(PlotTable_x1, PlotTable_y1(), PlotTable_x2, PlotTable_y2());
        }

        ImGui::PopStyleColor(); // Restore the previous style color
        ImGui::End();
    }

    ImGui::PopStyleColor();
}

void ShowFooter() {
    ImVec4 mainWindowBg = ImVec4(200.0f / 255.0f, 0.0f, 0.0f, 1.0f); 
    ImGui::PushStyleColor(ImGuiCol_WindowBg, mainWindowBg);
    if (ImGui::Begin("Footer", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar)) 
    {
        ImGui::SetWindowPos(ImVec2(0, ImGui::GetIO().DisplaySize.y - heightOfFooter)); 
        ImGui::SetWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x - widthOfSidebar, heightOfFooter));
        ImGui::Text("Welcome to the Footer Window!");
        ImGui::End();
    }
    ImGui::PopStyleColor();
}



// Main code
int layout()
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::Spectrum::StyleColorsSpectrum();
    // ImGui::Revolve::StyleColorsRevolve();
    //ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
#ifdef __EMSCRIPTEN__
    ImGui_ImplGlfw_InstallEmscriptenCanvasResizeCallback("#canvas");
#endif
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);

    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);
    io.Fonts->Clear();
    ImGui::Spectrum::LoadFont();

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.1f, 0.1f, 0.1f, 1.00f);




    // Main loop
#ifdef __EMSCRIPTEN__
    io.IniFilename = nullptr;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    while (!glfwWindowShouldClose(window))
#endif
    {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ShowNavbar();
        ShowSidebar();
        ShowMainWindow();
        ShowFooter();

        if (ImGui::Begin("plot8")) {
            data_manager.Plot8();   
            ImGui::End();
        }

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }
        

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif

    // Cleanup
    ImPlot::DestroyContext();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
