bifurcation_inc = include_directories('.')

bifurcation_deps = [
  fmt_dep,
]

bifurcation_lib = static_library(
  'bifurcation',
  'bifurcation_static.cpp',
  conf_h,
  dependencies: bifurcation_deps,
  include_directories: [bifurcation_inc, root_inc],
  install: false,
)

bifurcation_dep = declare_dependency(
  dependencies: bifurcation_deps,
  include_directories: bifurcation_inc,
  link_with: bifurcation_lib,
)
