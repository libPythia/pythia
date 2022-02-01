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

void large_loop(int * buffer, int buffer_size) {
#pragma omp parallel for
    for (int i = 0; i < buffer_size; i++) {
        for (int j = 0; j < buffer_size; j++) {
            buffer[i] = (buffer[i] * (j * j)) % buffer_size;
        }
    }
}

void init_buffer(int * buffer, int buffer_size) {
#pragma omp parallel for
    for (int i = 0; i < buffer_size; i++) {
        buffer[i] = (i + 1) % buffer_size;
    }
}

double gettick() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + (t.tv_nsec / 1e9);
}

struct loop_stat {
    int nthreads;
    int buffer_size;
    double min_duration;
    double max_duration;
    double total_duration;
    int nb_samples;
};

struct loop_stat * stats = NULL;
int nb_stats = 0;

struct loop_stat * find_stat(int nthreads, int buffer_size) {
    for (int i = 0; i < nb_stats; i++) {
        if (stats[i].nthreads == nthreads && stats[i].buffer_size == buffer_size)
            return &stats[i];
    }
    /* not found */
    nb_stats++;
    stats = realloc(stats, sizeof(struct loop_stat) * nb_stats);
    if (!stats) {
        fprintf(stderr, "cannot allocate memory\n");
        abort();
    }
    stats[nb_stats - 1].nthreads = nthreads;
    stats[nb_stats - 1].buffer_size = buffer_size;
    stats[nb_stats - 1].min_duration = 0;
    stats[nb_stats - 1].max_duration = 0;
    stats[nb_stats - 1].total_duration = 0;
    stats[nb_stats - 1].nb_samples = 0;

    return stats;
}

void update_stats(int nthreads, int buffer_size, double duration) {
    struct loop_stat * s = find_stat(nthreads, buffer_size);

    s->total_duration += duration;
    if (s->min_duration > duration || s->nb_samples == 0) {
        s->min_duration = duration;
    }
    if (s->max_duration < duration || s->nb_samples == 0) {
        s->max_duration = duration;
    }
    s->nb_samples++;
}

void print_stat(struct loop_stat * s, FILE * f) {
    if (s && s->nb_samples) {
        double avg = s->total_duration / s->nb_samples;
#define USEC(n) ((n)*1e6)
        fprintf(f,
                "%d,%d,%lf,%lf,%lf,%d\n",
                s->nthreads,
                s->buffer_size,
                USEC(avg),
                USEC(s->min_duration),
                USEC(s->max_duration),
                s->nb_samples);
    }
}

void print_stats() {
    FILE * f = fopen("bench_loop.csv", "w");
    fprintf(f, "#nthreads,buffer_size,avg,min,max,count\n");
    for (int i = 0; i < nb_stats; i++) {
        print_stat(&stats[i], f);
    }
    fclose(f);
}

int main() {
    int buffer_size_max = 1024 * 1024;
    // int buffer_size_max = 128;
    int nthreads_max = 16;
    omp_set_num_threads(nthreads_max);

    // int buffer_size = 1024;
    int * buffer = malloc(sizeof(int) * buffer_size_max);

    init_buffer(buffer, buffer_size_max);
    double t1, t2;

    for (int buffer_size = 1; buffer_size < buffer_size_max; buffer_size *= 2) {
        int niter;  // = (buffer_size_max/buffer_size);
        if (niter < 10000)
            niter = 10000;
        if (buffer_size > 32768)
            niter = 1000;
        int nthreads = 1;
        printf("#Buffer size: %d (niter=%d)\n", buffer_size, niter);

        for (nthreads = 1; nthreads <= nthreads_max; nthreads++) {
            //      printf("%d threads\n",nthreads);
            omp_set_num_threads(nthreads);
            for (int i = 0; i < niter; i++) {
                t1 = gettick();
                short_loop(buffer, buffer_size);
                t2 = gettick();
                update_stats(nthreads, buffer_size, t2 - t1);
            }
        }
    }

    print_stats();
    return 0;
}
