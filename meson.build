project(
  'google-ipmi-sys',
  'cpp',
  version: '0.1',
  meson_version: '>=0.63.0',
  default_options: [
    'cpp_std=c++20',
    'warning_level=3',
    'werror=true',
  ])

root_inc = include_directories('.')

conf_data = configuration_data()
conf_data.set_quoted('STATIC_BIFURCATION_CONFIG', get_option('static-bifurcation'))
conf_h = configure_file(
  output: 'config.h',
  configuration: conf_data)

bm_conf_data = configuration_data()
bm_conf_data.set10('BARE_METAL', get_option('bare_metal'))
bm_conf_data.set_quoted('BM_SIGNAL_PATH', get_option('bm-signal-path'))
bm_conf_h = configure_file(
  output: 'bm_config.h',
  configuration: bm_conf_data)

meson.get_compiler('cpp').has_header_symbol(
  'ipmid/api.h',
  'ipmid_get_sd_bus_connection')

json_dep = dependency('nlohmann_json', required: false)
meson.get_compiler('cpp').has_header_symbol(
  'nlohmann/json.hpp',
  'nlohmann::json',
  dependencies: json_dep)

fmt_dep = dependency('fmt', required: false)
if not fmt_dep.found()
  fmt_proj = import('cmake').subproject(
    'fmt',
    cmake_options: [
      '-DCMAKE_POSITION_INDEPENDENT_CODE=ON',
      '-DMASTER_PROJECT=OFF'
    ],
    required: false)
  assert(fmt_proj.found(), 'fmtlib is required')
  fmt_dep = fmt_proj.dependency('fmt')
endif

subdir('bifurcation')

sys_pre = declare_dependency(
  include_directories: root_inc,
  dependencies: [
    json_dep,
    fmt_dep,
    dependency('phosphor-dbus-interfaces'),
    dependency('phosphor-logging'),
    dependency('sdbusplus'),
    bifurcation_dep,
  ])

sys_lib = static_library(
  'sys',
  'bmc_mode.cpp',
  'cable.cpp',
  'cpld.cpp',
  'entity_name.cpp',
  'eth.cpp',
  'flash_size.cpp',
  'handler.cpp',
  'host_power_off.cpp',
  'ipmi.cpp',
  'machine_name.cpp',
  'pcie_i2c.cpp',
  'google_accel_oob.cpp',
  'pcie_bifurcation.cpp',
  'psu.cpp',
  'util.cpp',
  implicit_include_directories: false,
  dependencies: sys_pre)

sys_dep = declare_dependency(
  link_with: sys_lib,
  dependencies: sys_pre)

if not get_option('tests').disabled()
  subdir('test')
endif

shared_module(
  'googlesys',
  'main.cpp',
  implicit_include_directories: false,
  dependencies: [
    dependency('libipmid'),
    sys_dep,
  ],
  install: true,
  install_dir: get_option('libdir') / 'ipmid-providers')

systemd_dep = dependency('systemd')
if systemd_dep.found()
  install_data(
    'gbmc-host-poweroff.target',
    'gbmc-psu-hardreset.target',
    'gbmc-psu-hardreset-pre.target',
    'gbmc-psu-hardreset-time.service',
    install_dir: systemd_dep.get_variable(pkgconfig: 'systemdsystemunitdir'))
endif
