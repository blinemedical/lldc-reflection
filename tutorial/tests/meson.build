gtest_dep = dependency('gtest',
  fallback: ['gtest', 'gtest_dep'],
  include_type: 'system')

subdir('include')
subdir('src')

tutorial_test_deps = [
  libtutorial_dep,
  gtest_dep,
]

tutorial_test_exe = executable('tutorial-test', tutorial_test_src,
  include_directories: tutorial_test_inc,
  dependencies:        tutorial_test_deps,
  install:             false,
)

test('tutorial-test', tutorial_test_exe,
  protocol: 'gtest',
)
