# These should be defined prior to calling "subdir('test-template')"
# which results in the creation of a new test suite.  See the source
# file for expected preprocessor macros supported here.
#    test_template_name - the name to give to the test and executable
#    test_template_macro - One of the following values:
#      1. TEST_JSON_GLIB
#      2. TEST_SOCKET_IO

test_templates = []
if json_glib_dep.found()
  test_templates += [['TEST_JSON_GLIB', 'json-glib-test']]
endif

if sioclient_dep.found()
  test_templates += [['TEST_SOCKET_IO', 'socket-io-test']]
endif

foreach pair : test_templates
  test_template_macro = '-D' + pair[0] + '=1'
  test_template_name = pair[1]

  test_template_test_src = files(['test-template.cpp'])
  test_template_test_inc = include_directories('.')
  test_template_test_deps = test_deps
  test_template_test_cpp_args = test_cpp_args + test_template_macro
  test_template_test_link_args = test_link_args

  test_template_test_exe = executable(test_template_name,
    test_template_test_src,
    include_directories: test_template_test_inc,
    cpp_args: test_template_test_cpp_args,
    link_args: test_template_test_link_args,
    dependencies: test_template_test_deps,
    install: false,
  )

  test(test_template_name, test_template_test_exe)
endforeach
