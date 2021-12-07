gtest = dependency('gtest', main: true, disabler: true, required: get_option('tests'))
gmock = dependency('gmock', disabler: true, required: get_option('tests'))

tests_pre = declare_dependency(
  dependencies: [sys_dep, gtest, gmock])

tests_lib = static_library(
  'common',
  'common.cpp',
  'helper.cpp',
  implicit_include_directories: false,
  dependencies: tests_pre)

tests_dep = declare_dependency(
  link_with: tests_lib,
  dependencies: tests_pre)

tests = [
  'cable',
  'cpld',
  'entity',
  'eth',
  'flash',
  'handler',
  'linuxboot_boot_time',
  'machine',
  'pcie',
  'poweroff',
  'psu',
]

foreach t : tests
  test(
    t,
    executable(
      t.underscorify(),
      t + '_unittest.cpp',
      implicit_include_directories: false,
      dependencies: tests_dep))
endforeach
