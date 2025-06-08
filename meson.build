project(
    'foxconn-ipmi-oem',
    'cpp',
    version: '0.1',
    meson_version: '>=0.63.0',
    default_options: ['werror=true', 'warning_level=3', 'cpp_std=c++20'],
)

root_inc = include_directories('.', 'include')

# Dependencies
fiioemcmds_dep = declare_dependency(
    include_directories: root_inc,
    dependencies: [
        dependency('libipmid'),
        dependency('phosphor-logging'),
        dependency('sdbusplus'),
        dependency('libgpiod'),
    ],
)

shared_module(
    'fiioemcmds',
    'src/systemcommands.cpp',
    'src/file_handling.cpp',
    'src/bioscommands.cpp',
    implicit_include_directories: false,
    dependencies: fiioemcmds_dep,
    install: true,
    install_dir: get_option('libdir') / 'ipmid-providers',
)

