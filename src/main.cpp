#include <cstdio>

#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl2.h>
#include <imgui.h>

#include "App.h"

namespace {
void LogGlfwError(int error, const char* description) {
    std::FILE* file = std::fopen("startup.log", "a");
    if (file != nullptr) {
        std::fprintf(file, "GLFW error %d: %s\n", error, description != nullptr ? description : "(null)");
        std::fclose(file);
    }
    std::fprintf(stderr, "GLFW error %d: %s\n", error, description != nullptr ? description : "(null)");
}
} // namespace

int main() {
    if (std::FILE* file = std::fopen("startup.log", "w")) {
        std::fclose(file);
    }

    glfwSetErrorCallback(LogGlfwError);

    if (!glfwInit()) {
        const char* message = nullptr;
        const int code = glfwGetError(&message);
        std::fprintf(stderr, "Failed to initialize GLFW (%d): %s\n", code, message != nullptr ? message : "unknown");
        return 1;
    }

    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    GLFWwindow* window = glfwCreateWindow(1280, 820, "C++ Weather Studio", nullptr, nullptr);
    if (!window) {
        const char* message = nullptr;
        const int code = glfwGetError(&message);
        std::fprintf(stderr, "Failed to create GLFW window (%d): %s\n", code, message != nullptr ? message : "unknown");
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsLight();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL2_Init();

    App app;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        app.RenderFrame();

        ImGui::Render();
        int displayW = 0;
        int displayH = 0;
        glfwGetFramebufferSize(window, &displayW, &displayH);
        glViewport(0, 0, displayW, displayH);
        glClearColor(0.87f, 0.92f, 0.98f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
