project(
  'eta_factorizer',
  'cpp',
  version : '0.1',
  default_options : ['cpp_std=c++17', 'warning_level=3', 'b_ndebug=if-release']
)

pkgc = import('pkgconfig')

install_subdir('include/eta', install_dir:get_option('includedir'))

thread_dep = dependency('threads')

eta_core = subproject('eta_core').get_variable('eta_core_dep')
utils_dep = declare_dependency(include_directories:['src/utils'])

#----------------------------------------------------------------

eta_factorizer_src = [
    'src/factorization/bin_file.cpp',
    'src/factorization/check.cpp',
    'src/factorization/export.cpp',
    'src/factorization/flow_graph.cpp',
    'src/factorization/prediction.cpp',
    'src/factorization/reduction.cpp',
]

eta_factorizer_lib = static_library('eta_factorizer',
                               eta_factorizer_src,
                               include_directories:['include/eta/factorization'],
                               install:true)

eta_factorizer_dep = declare_dependency(include_directories:['include'],
                                   link_with:[eta_factorizer_lib])

#----------------------------------------------------------------

eta_oracle_lib = library('eta_oracle',
  [
    'src/oracle/predict_value.cpp',
  ],
  dependencies:[eta_factorizer_dep],
  include_directories:['include/eta/oracle'],
  install:true)

eta_oracle_dep = declare_dependency(include_directories:['include'], link_with:[eta_oracle_lib])


#----------------------------------------------------------------

eta_exe_src = [
  'src/eta/compare.cpp',
  'src/eta/input.cpp',
  'src/eta/main.cpp',
  'src/eta/settings.cpp',
]

eta_exe_deps = [eta_factorizer_dep, utils_dep, eta_core]

eta_exe = executable('eta_reduce',
  eta_exe_src,
  dependencies:eta_exe_deps,
  install_rpath:get_option('prefix')/get_option('libdir'),
  install:true)

#----------------------------------------------------------------

# pkgconfig export
pkgc.generate(eta_factorizer_lib,
  name: 'eta_factorizer',
  version: meson.project_version(),
  description: 'factorization module for EazyTrace Analyzer',
  extra_cflags:['-std=c++17'],
)

pkgc.generate(eta_oracle_lib,
  name: 'eta_oracle',
  version: meson.project_version(),
  description: 'oracle built on eta trace reduction algorithm',
)

#----------------------------------------------------------------

subdir('test')
