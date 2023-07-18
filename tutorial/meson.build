if host_machine.system() == 'windows'
  # Supporting windows would require doing all the declspec
  # decorating of the dynamic library so that the test can
  # link to the library, which is an exercise left for the
  # learner.  See how the lldc-reflection library does it,
  # which is based off JsonGlib and other similar libraries.
  error('The tutorial currently does not support Windows')
endif

subdir('lib')
subdir('tests')
