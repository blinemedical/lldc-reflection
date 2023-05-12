gtest_dep = dependency('gtest',
  fallback: ['gtest', 'gtest_dep'],
  include_type: 'system')

test_cpp_args = cpp_args
test_link_args = common_ldflags
test_deps = [gtest_dep]

subdir('common')
test_deps += common_test_lib_dep

subdir('test-template')
