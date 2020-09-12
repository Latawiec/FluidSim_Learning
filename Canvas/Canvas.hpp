#pragma once
#include <cassert>

namespace {
struct GLFWwindow;
}

struct Canvas {

    struct Color {
        float r, g, b;
    };

    Canvas(const int width, const int height, const char* windowName);
    ~Canvas();

    bool draw();

    void set_update_function(void(*)(Color*, const float));

private:
    GLFWwindow* window = nullptr;
    int width, height;
    float lastFrameTime = 0;
    int canvasProgram = -1;
    unsigned int vertexArrayObject = 0;
    unsigned int elementArrayBuffer = 0;
    unsigned int vertexBufferObject = 0;
    unsigned int canvasTexture = 0;
    unsigned int pixelBuffer = 0;
    void (*updateFunction)(Color*, const float) = nullptr;
};