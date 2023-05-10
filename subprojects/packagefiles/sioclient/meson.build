project('socket.io-client-cpp', 'cpp',
  version: '3.1.0',
  meson_version: '>= 0.63.0',
  default_options: [
    'cpp_std=c++17',
    'werror=false',
    'buildtype=debugoptimized',
  ],
)

# Websocketpp needs 11 but the closest we can do is either 14 or 17
# with MSVC and Meson.  This is related to the usage of 'result_of'
# and other features that have been removed in c++20.  Hence, the
# cpp_std=c++17 above.

sioclient_inc = include_directories(
  'src',
  'lib/asio/asio/include',
  'lib/websocketpp',
  'lib/rapidjson/include',
)

sioclient_cpp_args = [
  '-DASIO_STANDALONE',
  '-DBOOST_DATE_TIME_NO_LIB',
  '-DBOOST_REGEX_NO_LIB',
  '-D_WEBSOCKETPP_CPP11_FUNCTIONAL_',
  '-D_WEBSOCKETPP_CPP11_STL_'
]

sioclient_src = files(
  'src/sio_client.cpp',
  'src/sio_socket.cpp',
  'src/internal/sio_client_impl.cpp',
  'src/internal/sio_packet.cpp'
)

sioclient_lib = static_library('sioclient', sioclient_src,
  include_directories: sioclient_inc,
  cpp_args: sioclient_cpp_args,
)

sioclient_dep = declare_dependency(
  link_with: sioclient_lib,
  include_directories: sioclient_inc,
  compile_args: sioclient_cpp_args,
).as_system()

sioclient_tls_deps = []
sioclient_tls_deps += dependency('openssl', include_type: 'system')

sioclient_tls_cpp_args = sioclient_cpp_args
sioclient_tls_cpp_args += '-DSIO_TLS'

sioclient_tls_lib = static_library('sioclient_tls', sioclient_src,
  cpp_args: sioclient_tls_cpp_args,
  include_directories: sioclient_inc,
  dependencies: sioclient_tls_deps,
)

sioclient_tls_dep = declare_dependency(
  link_with: sioclient_tls_lib,
  include_directories: sioclient_inc,
  dependencies: sioclient_tls_deps,
  compile_args: sioclient_cpp_args,
).as_system()

# clean up
unset_variable('sioclient_src')
unset_variable('sioclient_inc')
unset_variable('sioclient_lib')
unset_variable('sioclient_tls_deps')
unset_variable('sioclient_tls_cpp_args')
unset_variable('sioclient_tls_lib')
