#define IX(i,j) ((i) + (N+2)*(j))
#define SWAP(x0, x) { float * tmp = x0, x0=x, x=tmp; }
#define FOR_EACH_CELL for (j=0; j < N ; j++) for (i=0; i < N; i++) {
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
    x[IX(0, 0)] =     0.5f * (x[IX(1, 0)] + x[IX(0, 1)]);
    x[IX(0, N+1)] =   0.5f * (x[IX(1, N+1)] + x[IX(0, N)]);
    x[IX(N+1, 0)] =   0.5f * (x[IX(N, 0)] + x[IX(N+1, 1)]);
    x[IX(N+1, N+1)] = 0.5f * (x[IX(N, N+1)] + x[IX(N+1, N)]);
}

void lin_solve(int N, int b, float *x, float *x0, float a, float c) {
    int i, j, n;
    for (n=0; n<20; n++) {
        FOR_EACH_CELL
            x[IX(i, j)] = (x0[IX(i,j)] + a*())
    }
}