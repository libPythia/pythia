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

glfw = dependency('glfw3', required:false)
glew = dependency('glew', required:false)
build_gui = glfw.found() and glew.found()

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

utils_dep = declare_dependency(include_directories:['src/utils'])

#----------------------------------------------------------------

eta_factorizer_bench = executable('bench',
                             'src/bench.cpp',
                             dependencies:[eta_factorizer_dep, utils_dep],
                            )

#----------------------------------------------------------------

eta_factorizer_exaustive_test = executable('exaustive_test',
                                  'src/exaustive_test.cpp',
                                  dependencies:[eta_factorizer_dep, thread_dep, utils_dep])

eta_factorizer_prediction_test = executable('prediction_test',
                                  'src/prediction_test.cpp',
                                  dependencies:[eta_factorizer_dep, thread_dep, utils_dep])

#----------------------------------------------------------------

if (build_gui)
    eta_gui_srcs = [
        'src/eta/gui/eta_gui.cpp',
        'src/eta/gui/imgui.cpp',
        'src/eta/gui/imgui_demo.cpp',
        'src/eta/gui/imgui_draw.cpp',
        'src/eta/gui/imgui_impl_glfw.cpp',
        'src/eta/gui/imgui_impl_opengl3.cpp',
        'src/eta/gui/imgui_tables.cpp',
        'src/eta/gui/imgui_widgets.cpp'
    ]

    eta_gui_lib = static_library('eta_reduce_gui',
        eta_gui_srcs,
        dependencies: [glfw, glew],
        install:false)

    eta_gui_dep = declare_dependency(
        link_with:eta_gui_lib,
        include_directories:'src/eta/gui',
        compile_args:'-DETA_GUI_ENABLED')
endif

#----------------------------------------------------------------

eta_exe_src = [
  'src/eta/compare.cpp',
  'src/eta/input.cpp',
  'src/eta/main.cpp',
  'src/eta/settings.cpp',
]

eta_exe_deps = [eta_factorizer_dep, utils_dep, eta_core]

if (build_gui)
    eta_exe_deps += eta_gui_dep
endif

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

