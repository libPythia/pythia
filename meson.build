project(
  'eta_trace',
  'cpp',
  version : '0.1',
  default_options : ['cpp_std=c++17', 'warning_level=3']
)

include_directory='include'

# For installation
install_subdir(include_directory, install_dir:get_option('includedir'))

# For embedding
if meson.get_compiler('cpp').get_id() == 'msvc'
    extra_args = ['-std:c++17', '/permissive-']
else
    extra_args = ['-std=c++17']
endif

eta_trace_lib = static_library('eta_trace',
                              ['src/trace.cpp'],
                              cpp_args:extra_args,
                              include_directories:[include_directory])

eta_trace_dep = declare_dependency(include_directories:[include_directory],
                                  link_with:[eta_trace_lib],
                                  compile_args:extra_args)

eta_trace_merge = executable('eta_trace_factorize',
                            'src/main.cpp',
                            dependencies:[eta_trace_dep])

# Testing
test_files=[
    'chunk_vector.cpp',
    'parents_tree.cpp',
    'trace.cpp',
    'vector.cpp',
]

foreach file : test_files
    src = 'test/' + file
    test_name = 'test_' + file.underscorify()
    test_exe = executable(test_name, src, dependencies:[eta_trace_dep], cpp_args:extra_args)
    test(test_name, test_exe)
endforeach


