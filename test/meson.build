gtest = dependency('gtest', main: true, disabler: true, required: get_option('tests'))
gmock = dependency('gmock', disabler: true, required: get_option('tests'))

tests = [
  'cable',
  'cpld',
  'entity',
  'eth',
  'flash',
  'handler',
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
      dependencies: [sys_dep, gtest, gmock]))
endforeach
