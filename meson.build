project('VERA Peer Library', 'cpp',
  version: '0.1.0',
  meson_version: '>= 0.63.0',
  default_options: [
    'cpp_std=c++20',
    'werror=true',
    'buildtype=debugoptimized'
  ]
)

# Get the c++ compiler wrapper for later.
cc = meson.get_compiler('cpp')
cmake = import('cmake')
host_system = host_machine.system()
cpp_args = []
extra_args = []
lldc_rttr_deps = []

cdata = configuration_data()
if get_option('default_library') != 'static'
  if host_system == 'windows'
    cdata.set('DLL_EXPORT', true)
    if cc.get_id() == 'msvc'
      cdata.set('LLDC_RTTR_EXPORT', '__declspec(dllexport)')
      cdata.set('LLDC_RTTR_IMPORT', '__declspec(dllimport)')
    else
      cdata.set('LLDC_RTTR_EXPORT', '__attribute__((visibility("default"))) __declspec(dllexport)')
      cdata.set('LLDC_RTTR_IMPORT', '__attribute__((visibility("default"))) __declspec(dllimport)')
      extra_args += ['-fvisibility=hidden']
    endif
  else
    cdata.set('LLDC_RTTR_EXPORT', '__attribute__((visibility("default")))')
    cdata.set('LLDC_RTTR_IMPORT', '__attribute__((visibility("default")))')
    extra_args += ['-fvisibility=hidden']
  endif
endif

extra_args += [
  '-DLLDC_RTTR_COMPILATION',
  '-DLLDC_RTTR_VERSION=@0@'.format(meson.project_version()),
]
common_ldflags = []

# RTTR reflection library
rttr_version = '0.9.6'
rttr_dep = dependency('rttr',
  version: '>=@0@'.format(rttr_version),
  required: false,
  include_type: 'system')

# Use the CMake module to import and configure it to build
# static libs, no installer/packager
if not rttr_dep.found()
  # Per RTTR docs, use -fno-gnu-unique with GCC.
  if cc.get_id() == 'gcc'
    cpp_args += '-fno-gnu-unique'
  endif

  rttr_defines = {
    'BUILD_DOCUMENTATION': false,
    'USE_PCH' : false,
    'CMAKE_BUILD_TYPE': 'Release',
  }

  # Configure any override options and defines
  rttr_opts = cmake.subproject_options()
  rttr_opts.set_override_option('werror', 'false')
  rttr_opts.add_cmake_defines(rttr_defines)

  # Configure the project with the options
  rttr_proj = cmake.subproject('rttr', options: rttr_opts)

  rttr_dep = rttr_proj.dependency('rttr_core', include_type: 'system')
endif
lldc_rttr_deps += rttr_dep

# Optional libs for the 'converters' namespace

# SocketIO-Client C++ (non-TLS)
sioclient_dep = dependency('sioclient',
  version: '>=3.1.0',
  include_type: 'system',
  required: false,
)
if not sioclient_dep.found() and get_option('socketio')
  sioclient = subproject('sioclient')
  sioclient_dep = sioclient.get_variable('sioclient_dep')
  lldc_rttr_deps += sioclient_dep
endif

# json-glib
json_glib_dep = dependency('json-glib-1.0',
  version: '>=1.6',
  include_type: 'system',
  fallback: ['json-glib-1.0', 'json_glib_dep'],
  required: get_option('jsonglib'),
)
if json_glib_dep.found()
  lldc_rttr_deps += json_glib_dep
endif

# Get sources, headers
subdir('include') # lldc_rttr_inc - public headers to install
subdir('src')     # lldc_rttr_src, lldc_rttr_priv_inc

# Create as a dynamic lib
lldc_rttr = library('lldc-rttr', lldc_rttr_src,
  include_directories: [lldc_rttr_inc, lldc_rttr_priv_inc],
  dependencies:        lldc_rttr_deps,
  cpp_args:            cpp_args + extra_args,
  link_args:           common_ldflags,
  install:             true)

lldc_rttr_dep = declare_dependency(
  link_with:           lldc_rttr,
  compile_args:        cpp_args,
  include_directories: lldc_rttr_inc,
  dependencies:        lldc_rttr_deps,
)

subdir('tests')
