project(
  'eta_factorizer',
  'cpp',
  version : '0.1',
  default_options : ['cpp_std=c++17', 'warning_level=3']
)

pkgc = import('pkgconfig')

install_subdir('include/eta', install_dir:get_option('includedir'))

eta_core = subproject('eta_core')
eta_core_dep = eta_core.get_variable('eta_core_dep')
thread_dep = dependency('threads')

eta_factorizer_src = [
    'src/export.cpp',
    'src/factorization.cpp',
    'src/parents_tree.cpp',
    'src/trace.cpp',
]

eta_factorizer_lib = static_library('eta_factorizer',
                               eta_factorizer_src,
                               dependencies:[eta_core_dep],
                               include_directories:['include/eta/factorization'],
                               install:true)

eta_factorizer_dep = declare_dependency(include_directories:['include'],
                                   link_with:[eta_factorizer_lib],
                                   dependencies:[eta_core_dep])

eta_factorizer_factorize = executable('factorize',
                                 'src/factorize.cpp',
                                 dependencies:[eta_factorizer_dep])

eta_factorizer_bench = executable('bench',
                             'src/bench.cpp',
                             dependencies:[eta_factorizer_dep],
                            )
eta_factorizer_fuzzy_test = executable('fuzzy_test',
                                  'src/fuzzy_test.cpp',
                                  dependencies:[eta_factorizer_dep, thread_dep])

eta_factorizer_fuzzy_bench = executable('fuzzy_bench',
                                   'src/fuzzy_bench.cpp',
                                   dependencies:[eta_factorizer_dep, thread_dep])

# Testing
test_files=[
    'parents_tree.cpp',
]

foreach file : test_files
    src = 'test/' + file
    test_name = 'test_' + file.underscorify()
    test_exe = executable(test_name, src, dependencies:[eta_factorizer_dep])
    test(test_name, test_exe)
endforeach

# pkgconfig export
pkgc.generate(eta_factorizer_lib,
  name: 'eta_factorizer',
  version: meson.project_version(),
  requires: ['nlc_meta', 'eta_core'],
  description: 'factorization module for EazyTrace Analyzer',
  extra_cflags:['-std=c++17'],
)

