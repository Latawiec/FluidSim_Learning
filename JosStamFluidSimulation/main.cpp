#include <iostream>

#include <Canvas.hpp>

constexpr unsigned int WIDTH = 800;
constexpr unsigned int HEIGHT = 600;

int main() {
    auto canvas = Canvas(WIDTH, HEIGHT, "Potato");
    canvas.set_update_function([](Canvas::Color* data) {
        data[100 * WIDTH + 100] = {1.f, 1.f, 1.f};
    });
    while(!canvas.draw()){}
}