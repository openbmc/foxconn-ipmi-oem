project(
  'foxconn-ipmi-oem',
  'cpp',
  version: '0.1',
  meson_version: '>=0.63.0',
  default_options: [
    'werror=true',
    'warning_level=3',
    'cpp_std=c++20',
  ]
)

root = meson.current_source_dir()
root_inc = include_directories('.', 'include')

# Project Arguments
cpp = meson.get_compiler('cpp')

# Dependencies
ipmid_dep = dependency('libipmid')
phosphor_logging_dep = dependency('phosphor-logging')
sdbusplus_dep = dependency('sdbusplus')
gpiodcxx_dep = dependency('libgpiod')

fiioemcmds_dep = declare_dependency(
  include_directories: root_inc,
  dependencies: [
    ipmid_dep,
    phosphor_logging_dep,
    sdbusplus_dep,
    gpiodcxx_dep,
  ]
)

library(
  'fiioemcmds',
  'src/systemcommands.cpp',
  'src/file_handling.cpp',
  'src/bioscommands.cpp',
  implicit_include_directories: false,
  dependencies: fiioemcmds_dep,
  version: meson.project_version(),
  override_options: ['b_lundef=false'],
  install: true,
  install_dir: get_option('libdir') / 'ipmid-providers'
)

