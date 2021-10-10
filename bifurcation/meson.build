bifurcation_inc = include_directories('.')

bifurcation_deps = [
  fmt_dep,
]

bifurcation_lib = static_library(
  'bifurcation',
  'bifurcation.cpp',
  dependencies: bifurcation_deps,
  include_directories: root_inc,
  version: meson.project_version(),
  install: false,
)

bifurcation_dep = declare_dependency(
  dependencies: bifurcation_deps,
  include_directories: root_inc,
  link_with: bifurcation_lib,
)
