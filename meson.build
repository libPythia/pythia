project(
  'eta_factorizer',
  'cpp',
  version : '0.1',
  default_options : ['cpp_std=c++17', 'warning_level=3', 'b_ndebug=if-release']
)

pkgc = import('pkgconfig')

install_subdir('include/eta', install_dir:get_option('includedir'))

thread_dep = dependency('threads')

eta_factorizer_src = [
    'src/check.cpp',
    'src/export.cpp',
    'src/factorization.cpp',
]

eta_factorizer_lib = static_library('eta_factorizer',
                               eta_factorizer_src,
                               include_directories:['include/eta/factorization'],
                               install:true)

eta_factorizer_dep = declare_dependency(include_directories:['include'],
                                   link_with:[eta_factorizer_lib])

eta_factorizer_factorize = executable('factorize',
                                 'src/factorize.cpp',
                                 dependencies:[eta_factorizer_dep])

eta_factorizer_bench = executable('bench',
                             'src/bench.cpp',
                             dependencies:[eta_factorizer_dep],
                            )

eta_factorizer_exaustive_test = executable('exaustive_test',
                                  'src/exaustive_test.cpp',
                                  dependencies:[eta_factorizer_dep, thread_dep])


# pkgconfig export
pkgc.generate(eta_factorizer_lib,
  name: 'eta_factorizer',
  version: meson.project_version(),
  description: 'factorization module for EazyTrace Analyzer',
  extra_cflags:['-std=c++17'],
)

