#include <iostream>
#include <random>
#include <algorithm>
#include <cstring>

#include <Canvas.hpp>
#include <Index2D.hpp>
#include <Neighbours2D.hpp>

// Jos Stam
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
    // auto x_2d = Marcin2D::make_index2d(x, x + N*N, N, N);
    // const auto x_2d_neighbourhood = Marcin2D::make_neighbours2d(x_2d);
    // const auto x0_2d = Marcin2D::make_index2d(x0, x0 + N*N, N, N);
    // int n = 20;
    // while (n-->0) {
    //     for(int y=1; y<N; ++y)
    //     for(int x=1; x<N; ++x) {
    //         const auto x_nn = x_2d_neighbourhood(x, y);
    //         x_2d(x, y) = (x0_2d(x, y) + a * (x_nn.left + x_nn.right + x_nn.top + x_nn.bot))/c;
    //     }
    //     set_bnd( N, b, x );
    // }
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
    //add_source(N, x0, x, dt);
    //SWAP(x0, x); 
    diffuse(N, 0, x, x0, diff, dt);
    SWAP(x0, x);
    advect(N, 0, x, x0, u, v, dt);
}

void vel_step(int N, float *u, float *v, float *u0, float *v0, float visc, float dt)
{
    //add_source(N, u0, u, dt);
    //add_source(N, v0, v, dt);
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
// 

constexpr unsigned int WIDTH = 300;
constexpr unsigned int HEIGHT = WIDTH;

float dens0[(WIDTH+2) * (HEIGHT+2)];
float dens[(WIDTH+2) * (HEIGHT+2)];
float vel_u0[(WIDTH+2) * (HEIGHT+2)];
float vel_v0[(WIDTH+2) * (HEIGHT+2)];
float vel_u[(WIDTH+2) * (HEIGHT+2)];
float vel_v[(WIDTH+2) * (HEIGHT+2)];


int main() {
    std::default_random_engine generator;
    std::uniform_real_distribution<float> distribution(-1.f, 1.f);

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
    
    auto canvas = Canvas(WIDTH, HEIGHT, "Potato");
    canvas.set_update_function([](Canvas::Color* data, const float dt) {
        std::cout << dt << " dt.\n";
        dens_step(WIDTH, dens, dens0, vel_u0, vel_v0, 0.000001f, dt);
        vel_step(WIDTH, vel_u, vel_v, vel_u0, vel_v0, 0.000001f, dt);
        for(int j=1; j<HEIGHT; ++j)
        for(int i=1; i<WIDTH; ++i) {
            const float density = dens[j * (WIDTH + 2) + i];
            const float velocity_v = std::clamp(std::abs(vel_v[j * (WIDTH + 2) + i]), 0.f, 1.f);
            const float velocity_u = std::clamp(std::abs(vel_u[j * (WIDTH + 2) + i]), 0.f, 1.f);
            data[j * WIDTH + i] = { density, density, density };
        }
    });

    while(!canvas.draw()){}
}