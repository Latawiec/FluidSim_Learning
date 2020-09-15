#include "Canvas.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
int create_canvas_program();

static float CANVAS_VERTICES[] = {
    // positions      // tex coords
    -1.f, -1.f, 0.f,  0.f, 1.f,
    1.f, -1.f, 0.f,   1.f, 1.f,
    1.f,  1.f, 0.f,   1.f, 0.f,
    -1.f,  1.f, 0.f,  0.f, 0.f
};

static int CANVAS_INDICES[] = {
    0, 2, 3, 0, 1, 2
};

Canvas::Canvas(const int width, const int height, const char* windowName)
: width(width),
  height(height)
{
    glfwInit() == GLFW_TRUE;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    this->window = glfwCreateWindow(width, height, windowName, NULL, NULL);
    assert(this->window != nullptr);

    glfwMakeContextCurrent(this->window);
    glfwSetFramebufferSizeCallback(this->window, framebuffer_size_callback);

    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    canvasProgram = create_canvas_program();
    glUseProgram(canvasProgram);
    glUniform1i(glGetUniformLocation(canvasProgram, "texture0"), 0);
    glUseProgram(0);

    glGenVertexArrays(1, &vertexArrayObject);
    glGenBuffers(1, &vertexBufferObject);
    glGenBuffers(1, &elementArrayBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementArrayBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(CANVAS_INDICES), CANVAS_INDICES, GL_STATIC_DRAW);
    glBindVertexArray(vertexArrayObject);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CANVAS_VERTICES), CANVAS_VERTICES, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(sizeof(float) * 3) );
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    glGenTextures(1, &canvasTexture);
    glBindTexture(GL_TEXTURE_2D, canvasTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    glGenBuffers(2, pixelBuffers);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pixelBuffers[0]);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, 3 * sizeof(float) * width * height, 0, GL_STREAM_DRAW);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pixelBuffers[1]);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, 3 * sizeof(float) * width * height, 0, GL_STREAM_DRAW);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);


    glClearColor(0.2f, 0.3f, 0.3f, 1.f);
    
    lastFrameTime = glfwGetTime();
}

Canvas::~Canvas() {
    glDeleteVertexArrays(1, &vertexArrayObject);
    glDeleteBuffers(2, pixelBuffers);
    glDeleteBuffers(1, &elementArrayBuffer);
    glDeleteBuffers(1, &vertexBufferObject);
    glDeleteProgram(canvasProgram);
    glfwTerminate();
}

static bool flipBuffer = false;
bool Canvas::draw() {
    glClear(GL_COLOR_BUFFER_BIT);
    const unsigned int readPixelBuffer = pixelBuffers[flipBuffer ? 1 : 0];
    const unsigned int writePixelBuffer = pixelBuffers[flipBuffer ? 0 : 1];

    glBindTexture(GL_TEXTURE_2D, canvasTexture);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, readPixelBuffer);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_FLOAT, 0);

    const float prevLastFrameTime = lastFrameTime;
    lastFrameTime = glfwGetTime();

    if (updateFunction != nullptr) {
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, writePixelBuffer);
        // From internet:
        // Note that glMapBuffer() causes sync issue.
        // If GPU is working with this buffer, glMapBuffer() will wait(stall)
        // until GPU to finish its job. To avoid waiting (idle), you can call
        // first glBufferData() with NULL pointer before glMapBuffer().
        // If you do that, the previous data in PBO will be discarded and
        // glMapBuffer() returns a new allocated pointer immediately
        // even if GPU is still working with the previous data.
        glBufferData(GL_PIXEL_UNPACK_BUFFER, 3 * sizeof(float) * width * height, 0, GL_STREAM_DRAW);
        Color* ptr = (Color*) glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
        if (ptr != nullptr) {
            updateFunction(ptr, lastFrameTime - prevLastFrameTime);
        } else {
            std::cout << "Did not map :( \n";
        }
        glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);            
    }
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    glActiveTexture(GL_TEXTURE0);
    glUseProgram(canvasProgram);
    
    glBindVertexArray(vertexArrayObject);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementArrayBuffer);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR) {
        std::cout << "Error: ";
        std::cout << err << '\n';
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
    flipBuffer = !flipBuffer;

    return glfwWindowShouldClose(window);
}

void Canvas::set_update_function(void(*function)(Color*, const float)) {
    this->updateFunction = function;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

int create_canvas_program()
{
    auto canvasVertSource = 
        #include "shaders/canvas.vert"
        ;
    int canvasVert = glCreateShader(GL_VERTEX_SHADER);

    glShaderSource(canvasVert, 1, &canvasVertSource, NULL);
    glCompileShader(canvasVert);
    {
        int success;
        glGetShaderiv(canvasVert, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(canvasVert, 512, NULL, infoLog);
            std::cout << "Failed to compile vertex shader:\n" << infoLog << std::endl;
            return -1;
        }
    }

    auto canvasFragSource = 
        #include "shaders/canvas.frag"
        ;
    int canvasFrag = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(canvasFrag, 1, &canvasFragSource, NULL);
    glCompileShader(canvasFrag);
    {
        int success;
        glGetShaderiv(canvasFrag, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(canvasFrag, 512, NULL, infoLog);
            std::cout << "Failed to compile fragment shader:\n" << infoLog << std::endl;
            return -1;
        }
    }

    int canvasProgram = glCreateProgram();
    glAttachShader(canvasProgram, canvasVert);
    glAttachShader(canvasProgram, canvasFrag);
    glLinkProgram(canvasProgram);
    {
        int success;
        glGetProgramiv(canvasProgram, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetProgramInfoLog(canvasFrag, 512, NULL, infoLog);
            std::cout << "Failed to link program:\n" << infoLog << std::endl;
            return -1;
        }
    }

    glDeleteShader(canvasFrag);
    glDeleteShader(canvasVert);

    return canvasProgram;
}