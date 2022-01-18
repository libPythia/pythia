omp = dependency('openmp', language: 'c')

bench_loop = executable('bench_loop', 'bench_loop.c', dependencies:omp)
bench_pythia = executable('bench_pythia', 'bench_pythia.c', dependencies:omp)
omp_set_numthread = executable('omp_set_numthread', 'omp_set_numthread.c', dependencies:omp)
