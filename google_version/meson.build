google_version_inc = include_directories('.')

google_version_deps = [
    dependency('libipmid'),
    dependency('phosphor-dbus-interfaces'),
    dependency('sdbusplus'),
    json_dep,
]

google_version_lib = static_library(
  'google_version',
  'google_version.cpp',
  dependencies: google_version_deps,
  include_directories: google_version_inc,
  version: meson.project_version(),
  install: false,
)

google_version_dep = declare_dependency(
  dependencies: google_version_deps,
  include_directories: google_version_inc,
  link_with: google_version_lib,
)
