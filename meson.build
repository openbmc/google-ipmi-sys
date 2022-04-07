project(
  'google-ipmi-sys',
  'cpp',
  version: '0.1',
  meson_version: '>=0.57.0',
  default_options: [
    'cpp_std=c++20',
    'warning_level=3',
    'werror=true',
  ])

meson.get_compiler('cpp').has_header_symbol(
  'ipmid/api.h',
  'ipmid_get_sd_bus_connection')

json_dep = dependency('nlohmann_json', required: false)
meson.get_compiler('cpp').has_header_symbol(
  'nlohmann/json.hpp',
  'nlohmann::json',
  dependencies: json_dep)

sys_pre = declare_dependency(
  include_directories: include_directories('.'),
  dependencies: [
    json_dep,
    dependency('phosphor-dbus-interfaces'),
    dependency('phosphor-logging'),
    dependency('sdbusplus'),
  ])

sys_lib = static_library(
  'sys',
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
