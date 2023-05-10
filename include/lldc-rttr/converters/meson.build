converters_header_dir = join_paths(install_header_dir, 'converters')

headers = []

if sioclient_dep.found()
  headers += 'socket-io.h'
endif

if json_glib_dep.found()
  headers += 'json-glib.h'
endif

install_headers(headers, install_dir: converters_header_dir)
unset_variable('converters_header_dir')
