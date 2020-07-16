{
  'variables': {
  },
  #'includes': ['../gypfiles/toolchain.gypi', '../gypfiles/features.gypi', 'inspector/inspector.gypi'],
  'targets': [
    {
      'target_name': 'v8',
      'dependencies_traverse': 1,
      'type': 'static_library',
      'dependencies': [
        'deps/jerryscript.gyp:jerrysource',
        'deps/jerryscript.gyp:jerry',
      ],
      'include_dirs+': [
        'deps/v8-headers',
        '<(DEPTH)',
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

        'src/v8jerry_callback.cpp',
        'src/v8jerry_callback.hpp',
        'src/v8jerry_context.hpp',
        'src/v8jerry_flags.cpp',
        'src/v8jerry_flags.hpp',
        'src/v8jerry_handlescope.cpp',
        'src/v8jerry_handlescope.hpp',
        'src/v8jerry_isolate.cpp',
        'src/v8jerry_isolate.hpp',
        'src/v8jerry_platform.hpp',
        'src/v8jerry_templates.cpp',
        'src/v8jerry_templates.hpp',
        'src/v8jerry_utils.cpp',
        'src/v8jerry_utils.hpp',
        'src/v8jerry_value.cpp',
        'src/v8jerry_value.hpp',

      ],
      'conditions': [
        ['want_separate_host_toolset==1', {
          'toolsets': ['host', 'target'],
        }, {
          'toolsets': ['target'],
        }],
        ['OS=="linux" and host_arch=="arm"', { # tizen
          'dependencies': [
            'deps/tizen-device-api/tizen_device_api.gyp:device_api',
          ],
        }],
      ]
    },
  ],
}
