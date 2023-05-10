install_headers([
    'api.h',
    'declaration.h',
    'registration.h',
  ],
  install_dir: install_header_dir
)

subdir('converters')
subdir('exceptions')
subdir('metadata')

# Generate the config.h from the cdata defined at the project root.
configure_file(output: 'config.h', configuration: cdata)
