project(
  'google-ipmi-sys',
  'cpp',
  version: '0.1',
  meson_version: '>=1.1.1',
  default_options: [
    'cpp_std=c++23',
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

stdplus = dependency('stdplus')
subdir('bifurcation')

sys_pre = declare_dependency(
  include_directories: root_inc,
  dependencies: [
    dependency('nlohmann_json', include_type: 'system'),
    dependency('phosphor-dbus-interfaces'),
    dependency('phosphor-logging'),
    dependency('sdbusplus'),
    stdplus,
    bifurcation_dep,
  ])

sys_lib = static_library(
  'sys',
  'bios_setting.cpp',
  'bm_instance.cpp',
  'bmc_mode.cpp',
  'cable.cpp',
  'cpld.cpp',
  'entity_name.cpp',
  'eth.cpp',
  'flash_size.cpp',
  'handler.cpp',
  'host_power_off.cpp',
  'ipmi.cpp',
  'linux_boot_done.cpp',
  'machine_name.cpp',
  'pcie_i2c.cpp',
  'google_accel_oob.cpp',
  'pcie_bifurcation.cpp',
  'file_system_wrapper.cpp',
  'psu.cpp',
  'util.cpp',
  implicit_include_directories: false,
  dependencies: sys_pre)

sys_dep = declare_dependency(
  link_with: sys_lib,
  dependencies: sys_pre)

if get_option('tests').allowed()
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
    install_dir: systemd_dep.get_variable('systemdsystemunitdir'))
endif

install_data(
  'gbmc-enqueue-powercycle.sh',
  install_dir: get_option('bindir'),
  install_mode: 'rwxr-xr-x'
)
