#include <iostream>
#include <vector>
#include <cstring>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "Index2D.hpp"
#include "Math.hpp"
#include <algorithm>
#include <random>


#define IX(i,j) ((i) + (N+2)*(j))
#define SWAP(x0, x) { float *tmp = x0; x0=x; x=tmp; }
#define FOR_EACH_CELL for (j=1; j < N ; j++) for (i=1; i < N; i++) {
#define END_FOR }

void add_source(int N, float *x, float *s, float dt) {
    int i, size = (N+2) * (N+2);
    for (i=0; i<size; ++i) 
        x[i] += dt * s[i];
}

void set_bnd(int N, int b, float *x) {
    int i;
    for (i=1; i<=N; ++i) {
        x[IX(0, i)] =   b == 1 ? -x[IX(1,i)] : x[IX(1,i)];
        x[IX(N+1, i)] = b == 1 ? -x[IX(N,i)] : x[IX(N,i)];
        x[IX(i, 0)] =   b == 2 ? -x[IX(i,1)] : x[IX(i,1)];
        x[IX(i, N+1)] = b == 2 ? -x[IX(i,N)] : x[IX(i,N)];
    }
    x[IX(0, 0)] =     0.5f * (x[IX(1, 0)]   + x[IX(0, 1)]);
    x[IX(0, N+1)] =   0.5f * (x[IX(1, N+1)] + x[IX(0, N)]);
    x[IX(N+1, 0)] =   0.5f * (x[IX(N, 0)]   + x[IX(N+1, 1)]);
    x[IX(N+1, N+1)] = 0.5f * (x[IX(N, N+1)] + x[IX(N+1, N)]);
}

void lin_solve(int N, int b, float *x, float *x0, float a, float c) {
    int i, j, n;
    for (n=0; n<20; n++) {
        FOR_EACH_CELL
            x[IX(i, j)] = (x0[IX(i,j)] + a*(x[IX(i-1, j)] + 
                x[IX(i+1,j)] + x[IX(i, j-1)] + x[IX(i, j+1)]))/c;
        END_FOR
        set_bnd( N, b, x );
    }
}

void diffuse(int N, int b, float *x, float *x0, float diff, float dt)
{
    float a=dt*diff*N*N;
    lin_solve(N, b, x, x0, a, 1+4*a);
}

void advect(int N, int b, float *d, float *d0, float *u, float *v, float dt)
{
    int i, j, i0, j0, i1, j1;
    float x, y, s0, t0, s1, t1, dt0;
    dt0=dt*N;

    FOR_EACH_CELL
        x = i-dt0*u[IX(i,j)];
        y = j-dt0*v[IX(i,j)];

        if (x < 0.5f) x = 0.5f;
        if (x > N + 0.5f) x = N + 0.5f;
        i0 = (int)x;
        i1 = i0 + 1;

        if (y < 0.5f) y = 0.5f;
        if (y > N + 0.5f) y = N + 0.5f;
        j0 = (int)y;
        j1 = j0 + 1;

        s1 = x - i0;
        s0 = 1 - s1;
        t1 = y - j0;
        t0 = 1 - t1;
        
        d[IX(i,j)] = s0 * (t0 * d0[IX(i0,j0)] + t1*d0[IX(i0,j1)]) +
                     s1 * (t0 * d0[IX(i1,j0)] + t1*d0[IX(i1,j1)]);
    END_FOR
    set_bnd(N, b, d);
}

void project(int N, float *u, float *v, float *p, float *div)
{
    int i, j;
    FOR_EACH_CELL
        div[IX(i,j)] = -0.5f * (u[IX(i+1, j)] - u[IX(i-1, j)] + v[IX(i, j+1)] - v[IX(i, j-1)]) / N;
        p[IX(i,j)] = 0;
    END_FOR

    set_bnd(N, 0, div);
    set_bnd(N, 0, p);
    lin_solve(N, 0, p, div, 1, 4);

    FOR_EACH_CELL
        u[IX(i,j)] -= 0.5f * N * (p[IX(i+1,j)] - p[IX(i-1,j)]);
        v[IX(i,j)] -= 0.5f * N * (p[IX(i,j+1)] - p[IX(i,j-1)]);
    END_FOR
    set_bnd(N, 1, u);
    set_bnd(N, 2, v);
}

void dens_step(int N, float *x, float *x0, float *u, float *v, float diff, float dt)
{
    add_source(N, x0, x, dt);
    //SWAP(x0, x); 
    diffuse(N, 0, x, x0, diff, dt);
    SWAP(x0, x);
    advect(N, 0, x, x0, u, v, dt);
}

void vel_step(int N, float *u, float *v, float *u0, float *v0, float visc, float dt)
{
    add_source(N, u0, u, dt);
    add_source(N, v0, v, dt);
    // SWAP(u0, u);
    // SWAP(v0, v);
    
    diffuse(N, 1, u, u0, visc, dt);
    diffuse(N, 2, v, v0, visc, dt);
    project(N, u, v, u0, v0);

    SWAP(u0, u);
    SWAP(v0, v);
    advect(N, 1, u, u0, u0, v0, dt);
    advect(N, 2, v, v0, u0, v0, dt);
    project(N, u, v, u0, v0);
}


// Now some drawing.

constexpr unsigned int WIDTH = 400;
constexpr unsigned int HEIGHT = WIDTH;


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
int create_canvas_program();

float dens0[(WIDTH+2) * (HEIGHT+2)];
float dens[(WIDTH+2) * (HEIGHT+2)];
float vel_u0[(WIDTH+2) * (HEIGHT+2)];
float vel_v0[(WIDTH+2) * (HEIGHT+2)];
float vel_u[(WIDTH+2) * (HEIGHT+2)];
float vel_v[(WIDTH+2) * (HEIGHT+2)];

void update(float* out, const float dt) {
    dens_step(WIDTH, dens, dens0, vel_u0, vel_v0, 0.000001f, dt);
    vel_step(WIDTH, vel_u, vel_v, vel_u0, vel_v0, 0.000001f, dt);
    for(int j=1; j<HEIGHT; ++j)
    for(int i=1; i<WIDTH; ++i) {
        out[3*(j * WIDTH + i)]     = dens[j * (WIDTH + 2) + i];
        out[3*(j * WIDTH + i) + 1] = dens[j * (WIDTH + 2) + i];
        out[3*(j * WIDTH + i) + 2] = dens[j * (WIDTH + 2) + i];
    }
}

void checkErrors(const char* label) {
    {
        GLenum err;
        while((err = glGetError()) != GL_NO_ERROR) {
            std::cout << "Error@" << label << ": ";
            std::cout << err << '\n';
        }
    }
}

std::default_random_engine generator;
std::uniform_real_distribution<float> distribution(-100.f, 100.f);

int main() {

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(2*WIDTH, 2*HEIGHT, "Jos Stam Fluid Simulation", NULL, NULL);

    if (window == nullptr) {
        std::cout << "Failed to create GLFW window.\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);


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

    // Init
    memset(dens, 0,   sizeof(dens));
    memset(dens0, 0,  sizeof(dens0));
    memset(vel_u, 0,  sizeof(vel_u));
    memset(vel_u0, 0, sizeof(vel_u0));
    memset(vel_v, 0,  sizeof(vel_v));
    memset(vel_v0, 0, sizeof(vel_v0));

    for(int j=1; j<HEIGHT; ++j)
    for(int i=1; i<WIDTH; ++i) {
        const int offset = WIDTH/2;
        if ( i > offset-30 && i < offset+50 && j > offset-30 && j < offset+50) {
            dens0[j * (WIDTH + 2) + i] = 1.f;
        }

        if ( i > 50 && i < WIDTH-50 && j > 50 && j < HEIGHT-50) {
            vel_u0[j * (WIDTH + 2) + i] = distribution(generator);
            vel_v0[j * (WIDTH + 2) + i] = distribution(generator);
        }
    }
    

    //
    double lastFrameTime = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        
        glClearColor(0.2f, 0.3f, 0.3f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindTexture(GL_TEXTURE_2D, outputTexture);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pixelBuffer);
        float* ptr = (float*) glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
        if (ptr != nullptr) {
            update(ptr, glfwGetTime() - lastFrameTime);
        } else {
            std::cout << "Could not map buffer.\n";
        }
        glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT, GL_RGB, GL_FLOAT, 0);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, outputTexture);
        glUseProgram(canvasProgram);
        
        glBindVertexArray(VAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();

        lastFrameTime = glfwGetTime();

        checkErrors("End Loop");
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

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        
        vel_u0[(HEIGHT - (int)ypos/2) * (WIDTH + 2) + (int)xpos/2] = 20.f * distribution(generator);
        vel_v0[(HEIGHT - (int)ypos/2) * (WIDTH + 2) + (int)xpos/2] = 20.f * distribution(generator);

    }
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