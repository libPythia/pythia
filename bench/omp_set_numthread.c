#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void short_loop(int * buffer, int buffer_size) {
#pragma omp parallel for
    for (int i = 0; i < buffer_size; i++) {
        buffer[i] = (buffer[i] * (i * i)) % buffer_size;
    }
}

double gettick() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + (t.tv_nsec / 1e9);
}

int main(int argc, char ** argv) {
    int buffer_size = 1024;
    int * buffer = malloc(sizeof(int) * buffer_size);

    int n1 = 2;
    int n2 = 4;
    if (argc >= 3) {
        n1 = atoi(argv[1]);
        n2 = atoi(argv[2]);
    }

    int niter = 10000;

    double t1, t2, t3;

    omp_set_num_threads(n1);
#pragma omp parallel for
    for (int i = 0; i < buffer_size; i++) {
        buffer[i] = i + 1;
    }

    double t_change = 0;
    double t_nochange = 0;
    for (int i = 0; i < niter; i++) {
        omp_set_num_threads(n1);
        short_loop(buffer, buffer_size);

        t1 = gettick();
        omp_set_num_threads(n2);
        short_loop(buffer, buffer_size);
        t2 = gettick();
        short_loop(buffer, buffer_size);
        t3 = gettick();

        t_change += t2 - t1;
        t_nochange += t3 - t2;
    }

    printf("while changing from %d to %d: %lf ns\n", n1, n2, t_change * 1e9 / niter);
    printf("without changing from %d to %d: %lf ns\n", n1, n2, t_nochange * 1e9 / niter);
    double avg = (t_change - t_nochange) / niter;
    printf("changing from %d to %d costs %lf ns\n", n1, n2, avg * 1e9);
    return 0;
}
