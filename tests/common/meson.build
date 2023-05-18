common_test_extra_args = [ '-DCOMMON_TEST_COMPILATION' ]
common_test_cpp_args = test_cpp_args
common_test_link_args = test_link_args

cc = meson.get_compiler('cpp')
host_system = host_machine.system()
common_test_data = configuration_data()
if get_option('default_library') != 'static'
  if host_system == 'windows'
    common_test_data.set('DLL_EXPORT', true)
    if cc.get_id() == 'msvc'
      common_test_data.set('COMMON_TEST_EXPORT', '__declspec(dllexport)')
      common_test_data.set('COMMON_TEST_IMPORT', '__declspec(dllimport)')
    else
      common_test_data.set('COMMON_TEST_EXPORT', '__attribute__((visibility("default"))) __declspec(dllexport)')
      common_test_data.set('COMMON_TEST_IMPORT', '__attribute__((visibility("default"))) __declspec(dllimport)')
      common_test_extra_args += ['-fvisibility=hidden']
    endif
  else
    common_test_data.set('COMMON_TEST_EXPORT', '__attribute__((visibility("default")))')
    common_test_data.set('COMMON_TEST_IMPORT', '__attribute__((visibility("default")))')
    common_test_extra_args += ['-fvisibility=hidden']
  endif
endif

common_test_deps = [lldc_reflection_dep]
common_test_src  = files([
  'src/example_messages.cpp'
])
common_test_inc  = include_directories('.')

subdir('common')

common_test_lib = shared_library('common-test', common_test_src,
  include_directories: common_test_inc,
  dependencies:        common_test_deps,
  cpp_args:            common_test_cpp_args + common_test_extra_args,
  link_args:           common_test_link_args,
  install:             false,
)

common_test_lib_dep = declare_dependency(
  link_with:           common_test_lib,
  compile_args:        common_test_cpp_args,
  include_directories: common_test_inc,
  dependencies:        common_test_deps,
)

# Add the library to the root path if using vs backend because of apparent
# bug in meson.
if meson.backend().startswith('vs')
  lib_path = environment()
  lib_path.prepend('PATH', meson.current_build_dir())
  meson.add_devenv(lib_path, method: 'prepend')
endif

unset_variable('common_test_data')
unset_variable('common_test_lib')
unset_variable('common_test_inc')
unset_variable('common_test_src')
unset_variable('common_test_deps')
