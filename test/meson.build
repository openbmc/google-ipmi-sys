gtest = dependency(
    'gtest',
    main: true,
    disabler: true,
    required: get_option('tests'),
)
gmock = dependency('gmock', disabler: true, required: get_option('tests'))

tests_pre = declare_dependency(
    dependencies: [sys_dep, gtest, gmock, dependency('stdplus-gtest')],
)

tests_lib = static_library(
    'common',
    'common.cpp',
    'helper.cpp',
    implicit_include_directories: false,
    dependencies: tests_pre,
)

tests_dep = declare_dependency(link_with: tests_lib, dependencies: tests_pre)

tests = [
    'cable',
    'cpld',
    'entity',
    'eth',
    'flash',
    'google_accel_oob',
    'handler',
    'machine',
    'pcie',
    'poweroff',
    'psu',
    'pcie_bifurcation',
    'bmc_mode',
    'linux_boot_done',
    'bm_mode_transition',
    'bm_instance',
    'bios_setting',
]

foreach t : tests
    test(
        t,
        executable(
            t.underscorify(),
            t + '_unittest.cpp',
            implicit_include_directories: false,
            dependencies: tests_dep,
        ),
    )
endforeach
