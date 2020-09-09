#include <iostream>
#include <vector>
#include <cstring>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "Index2D.hpp"
#include "Math.hpp"


constexpr unsigned int WIDTH = 800;
constexpr unsigned int HEIGHT = 600;


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
int create_canvas_program();
void update_data(float data[], int width, int height);

std::vector<glm::vec3> currentState;
std::vector<glm::vec3> nextState;

std::vector<glm::vec2> currentVelocityField;
std::vector<glm::vec2> nextVelocityField;

std::vector<float> pressure, divergence;

glm::vec2 operator/(const glm::vec2 val, const float div) {
    return { val[0]/div, val[1]/div };
}

int main() {

    currentState.resize(WIDTH * HEIGHT);
    nextState.resize(WIDTH * HEIGHT);
    currentVelocityField.resize(WIDTH * HEIGHT);
    nextVelocityField.resize(WIDTH * HEIGHT);
    pressure.resize(WIDTH * HEIGHT);
    divergence.resize(WIDTH * HEIGHT);

    auto indexedCurrent = Marcin2D::make_index2d(std::begin(currentState), std::end(currentState), WIDTH, HEIGHT);
    for (int y=0; y<HEIGHT; ++y)
    for (int x=0; x<WIDTH; ++x) {
        const auto position = glm::vec2{ x, y };
        const auto center = glm::vec2{ WIDTH/2, HEIGHT/2 };
        const auto treshold = 100.f;
        if (glm::distance(position, center) < treshold) {
            indexedCurrent(x, y) = {1.f, 1.f, 1.f};
        }
    }

    auto indexedVelocityField = Marcin2D::make_index2d(std::begin(currentVelocityField), std::end(currentVelocityField), WIDTH, HEIGHT);
    for (int y=0; y<HEIGHT; ++y)
    for (int x=0; x<WIDTH; ++x) {
        indexedVelocityField(x, y) = { 0.f, 0.f };
    }

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Jos Stam Fluid Simulation", NULL, NULL);

    if (window == nullptr) {
        std::cout << "Failed to create GLFW window.\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);


    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD.\n";
        return -1;
    }

    // Actual good stuff
    auto canvasProgram = create_canvas_program();
    glUseProgram(canvasProgram);
    glUniform1i(glGetUniformLocation(canvasProgram, "texture0"), 0);
    glUseProgram(0);

    float vertices[] = {
        // positions   // tex coords
        -1.f, -1.f, 0.f,  0.f, 0.f,
         1.f, -1.f, 0.f,  1.f, 0.f,
         1.f,  1.f, 0.f,  1.f, 1.f,
        -1.f,  1.f, 0.f,  0.f, 1.f
    };

    int indices[] = {
        0, 1, 2, 0, 2, 3
    };
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(sizeof(float) * 3) );
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    unsigned int outputTexture;
    glGenTextures(1, &outputTexture);
    glBindTexture(GL_TEXTURE_2D, outputTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);

    unsigned int pixelBuffer;
    glGenBuffers(1, &pixelBuffer);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pixelBuffer);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, 3 * sizeof(float) * WIDTH * HEIGHT, 0, GL_STREAM_DRAW);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    //
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.2f, 0.3f, 0.3f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindTexture(GL_TEXTURE_2D, outputTexture);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pixelBuffer);
        float* ptr = (float*) glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
        if (ptr != nullptr) {
            
            update_data(ptr, WIDTH, HEIGHT);
            
            glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);            
        } else {
            std::cout << "Did not map :( \n";
        }
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pixelBuffer);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT, GL_RGB, GL_FLOAT, 0);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, outputTexture);
        glUseProgram(canvasProgram);
        
        glBindVertexArray(VAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        GLenum err;
        while((err = glGetError()) != GL_NO_ERROR) {
            std::cout << "Error: ";
            std::cout << err << '\n';
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(canvasProgram);

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

int create_canvas_program() {
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

void update_data(float* data, int width, int height) {
    auto indexedData = Marcin2D::make_index2d(data, data + width * height, width, height);

    gauss_seidel(width, height, currentState, nextState, 0.2f);
    semi_lagrarian(width, height, currentVelocityField, nextState, currentState, 0.5f);

    // vel step
    {
        auto indexedCurrentVelocity = Marcin2D::make_index2d(std::begin(currentVelocityField), std::end(currentVelocityField), width, height);
        for (int y=0; y<HEIGHT; ++y)
        for (int x=0; x<WIDTH; ++x) {
            if ( (x < WIDTH/2 + 10) && (x > WIDTH/2 - 10) ) {
                indexedCurrentVelocity(x, y) = {10.5f, 10.5f};
            }
        }
        gauss_seidel(width, height, currentVelocityField, nextVelocityField, 0.2f);
        project(width, height, nextVelocityField, pressure, divergence);
        semi_lagrarian(width, height, nextVelocityField, nextVelocityField, currentVelocityField, 0.2f);
        project(width, height, nextVelocityField, pressure, divergence);
    }

    memcpy(data, currentState.data(), width * height * sizeof(glm::vec3));
}