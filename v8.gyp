{
  'variables': {
  },
  #'includes': ['../gypfiles/toolchain.gypi', '../gypfiles/features.gypi', 'inspector/inspector.gypi'],
  'targets': [
    {
      'target_name': 'v8',
      'type': 'static_library',
      'dependencies': ['deps/jerry/jerry.gyp:jerry'],
      'include_dirs+': [
        'deps/v8-headers',
        'deps/jerry',
        '<(DEPTH)',
        '<(SHARED_INTERMEDIATE_DIR)'
      ],
      'direct_dependent_settings': {
        'defines': [ 'V8JERRY' ],
        'include_dirs': [ 'deps/v8-headers' ]
      },
      'sources': [  ### gcmole(all) ###
        'deps/v8-headers/v8-debug.h',
        'deps/v8-headers/v8-platform.h',
        'deps/v8-headers/v8-profiler.h',
        'deps/v8-headers/v8-testing.h',
        'deps/v8-headers/v8-util.h',
        'deps/v8-headers/v8-value-serializer-version.h',
        'deps/v8-headers/v8-version-string.h',
        'deps/v8-headers/v8-version.h',
        'deps/v8-headers/v8.h',
        'deps/v8-headers/v8config.h',

        'src/v8jerry.cpp',
      ],
      'conditions': [
        ['want_separate_host_toolset==1', {
          'toolsets': ['host', 'target'],
        }, {
          'toolsets': ['target'],
        }],
      ]
    },
  ],
}
