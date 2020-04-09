project(
  'eta_trace',
  'cpp',
  version : '0.1',
  default_options : ['cpp_std=c++17', 'warning_level=3']
)

thread_dep = dependency('threads')

include_directory='include'

# For installation
install_subdir(include_directory, install_dir:get_option('includedir'))

eta_trace_src = [
    'src/dot_export.cpp',
    'src/factorization.cpp',
    'src/parents_tree.cpp',
    'src/trace.cpp',
    'src/trace_to_string.cpp',
]

eta_core = subproject('eta_core')
eta_core_dep = eta_core.get_variable('eta_core_dep')

eta_trace_lib = static_library('eta_trace',
                               eta_trace_src,
                               dependencies:[eta_core_dep],
                               include_directories:[include_directory])

eta_trace_dep = declare_dependency(include_directories:[include_directory],
                                   link_with:[eta_trace_lib],
                                   dependencies:[eta_core_dep])

eta_trace_factorize = executable('factorize',
                                 'src/factorize.cpp',
                                 dependencies:[eta_trace_dep])

eta_trace_bench = executable('bench',
                             'src/bench.cpp',
                             dependencies:[eta_trace_dep],
                            )
eta_trace_fuzzy_test = executable('fuzzy_test',
                                  'src/fuzzy_test.cpp',
                                  dependencies:[eta_trace_dep, thread_dep])

eta_trace_fuzzy_bench = executable('fuzzy_bench',
                                   'src/fuzzy_bench.cpp',
                                   dependencies:[eta_trace_dep, thread_dep])

# Testing
test_files=[
    'parents_tree.cpp',
]

foreach file : test_files
    src = 'test/' + file
    test_name = 'test_' + file.underscorify()
    test_exe = executable(test_name, src, dependencies:[eta_trace_dep])
    test(test_name, test_exe)
endforeach


