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
add_project_arguments(
  cpp.get_supported_arguments([
    '-DBOOST_ERROR_CODE_HEADER_ONLY',
    '-DBOOST_SYSTEM_NO_DEPRECATED',
    '-DBOOST_ALL_NO_LIB',
    '-DBOOST_NO_RTTI',
    '-DBOOST_NO_TYPEID',
    '-DBOOST_ASIO_DISABLE_THREADS',
    '-DBOOST_COROUTINES_NO_DEPRECATION_WARNING',
    '-Wno-psabi',
  ]),
  language : 'cpp'
)

# Dependencies
ipmid = dependency('libipmid')
phosphor_logging_dep = dependency('phosphor-logging')
crypto = dependency('libcrypto', version : '>=1.0.2g')
sdbusplus_dep = dependency('sdbusplus')
std_fs = cpp.find_library('stdc++fs')
userlayer = cpp.find_library('userlayer')
channellayer = cpp.find_library('channellayer')
gpiodcxx = cpp.find_library('gpiodcxx')

fiioemcmds_dep = declare_dependency(
  include_directories: root_inc,
  dependencies: [
    crypto,
    ipmid,
    phosphor_logging_dep,
    sdbusplus_dep,
    std_fs,
    userlayer,
    channellayer,
    gpiodcxx,
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

