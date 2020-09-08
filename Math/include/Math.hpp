#pragma once
#include <vector>
#include <tuple>
#include "Math.hpp"
#include "Neighbours2D.hpp"
#include "Index2D.hpp"
#include "Wrap2D.hpp"
#include "Bilinear.hpp"

template<class T>
void gauss_seidel(
    const int width,
    const int height,
    const std::vector<T>& currentState,
    std::vector<T>& nextState,
    const float weight
) 
{
    using namespace Marcin2D;
    constexpr int repeats = 20;

    const auto indexCurrent = make_index2d(std::begin(currentState), std::end(currentState), width, height);
    auto indexNext = make_wrap2d<WrapMode::Clamp>(make_index2d(std::begin(nextState), std::end(nextState), width, height));
    auto neighbourhoodNext = make_neighbours2d(indexNext);

    for (int i=0; i<repeats; ++i) {
        for(int y = 0; y < height; ++y)
        for(int x = 0; x < width; ++x) {
            const auto& nextStateNeighbours = neighbourhoodNext(x, y);
            indexNext(x, y) = ( indexCurrent(x,y) + weight * ( nextStateNeighbours.bot + nextStateNeighbours.top + nextStateNeighbours.left + nextStateNeighbours.right ) ) / (1.f + 4*weight);
        }
    }
}

template<class T, class K>
void semi_lagrarian(
    const int width,
    const int height,
    const std::vector<T>& velocity,
    const std::vector<K>& currentState,
    std::vector<K>& nextState,
    const float dt
)
{
    using namespace Marcin2D;
    const auto indexVelocity = make_index2d(std::begin(velocity), std::end(velocity), width, height);
    const auto indexCurrentState = make_index2d(std::begin(currentState), std::end(currentState), width, height);
    const auto bilinearCurrentState = make_bilinear(make_wrap2d<WrapMode::Clamp>(indexCurrentState));
    auto indexNextState = make_index2d(std::begin(nextState), std::end(nextState), width, height);

    for (int y=0; y < height; ++y)
    for (int x=0; x < width; ++x) {
        const auto velocity = indexVelocity(x, y);
        const auto [x_moved, y_moved] = std::make_tuple(x + velocity.x * -dt, y + velocity.y * -dt);

        indexNextState(x, y) = bilinearCurrentState(x_moved, y_moved);
    }
}

template<class T>
void project(
    const int width,
    const int height,
    std::vector<T>& velocity,
    std::vector<float>& pressure,
    std::vector<float>& divergence
)
{
    using namespace Marcin2D;
    auto divergence2D = make_index2d(std::begin(divergence), std::end(divergence), width, height);
    auto velocity2D = make_index2d(std::begin(velocity), std::end(velocity), width, height);
    auto pressure2D = make_index2d(std::begin(pressure), std::end(pressure), width, height);

    const auto velocity2D_clamped = make_wrap2d<WrapMode::Clamp>(velocity2D);
    const auto velocityNeighbours = make_neighbours2d(velocity2D_clamped);

    for (int y=0; y < height; y++)
    for (int x=0; x < width; x++) {
        const auto v_n = velocityNeighbours(x, y);
        divergence2D(x, y) = -0.5f * (v_n.right[0] - v_n.left[0]) / width + (v_n.bot[1] - v_n.top[1]) / height;
        pressure2D(x, y) = 0;
    }

    gauss_seidel(width, height, divergence, pressure, 1.f);

    const auto pressureNeighbours = make_neighbours2d(make_wrap2d<WrapMode::Clamp>(pressure2D));

    for (int y=0; y < height; y++)
    for (int x=0; x < width; x++) {
        const auto p_n = pressureNeighbours(x, y);
        velocity2D(x, y)[0] -= 0.5f * width * (p_n.right - p_n.left);
        velocity2D(x, y)[1] -= 0.5f * height * (p_n.bot - p_n.top);
    }
}

