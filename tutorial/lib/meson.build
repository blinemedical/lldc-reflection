subdir('include')
subdir('src')

libtutorial_deps = [lldc_reflection_dep.as_system()]

libtutorial = shared_library ('tutorial', libtutorial_src,
  include_directories: libtutorial_inc,
  dependencies:        libtutorial_deps,
  install:             false)

libtutorial_dep = declare_dependency(
  link_with:           libtutorial,
  include_directories: libtutorial_inc,
  dependencies:        libtutorial_deps,
)
