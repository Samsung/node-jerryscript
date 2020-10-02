{
  'variables': {
    'generated_sources': [
      '<(SHARED_INTERMEDIATE_DIR)/jerry/jerryscript.c',
      '<(SHARED_INTERMEDIATE_DIR)/jerry/jerryscript-config.h',
      '<(SHARED_INTERMEDIATE_DIR)/jerry/jerryscript.h',
      '<(SHARED_INTERMEDIATE_DIR)/jerry/jerryscript-port-default.c',
      '<(SHARED_INTERMEDIATE_DIR)/jerry/jerryscript-port-default.h',
    ]
   },
  'targets': [
    {
      'target_name': 'jerrysource',
      'type': 'none',
      'actions': [
        {
          'action_name': 'jerrysource',
          'inputs': [
            'jerryscript/tools/srcgenerator.py',
          ],
          'outputs': [
            '<@(generated_sources)',
          ],
          'action': [
            'python',
            'jerryscript/tools/srcgenerator.py',
            '--output-dir=<(SHARED_INTERMEDIATE_DIR)/jerry/',
            '--jerry-core',
            '--jerry-port-default',
          ],
        },
      ],
    },
    {
      'target_name': 'jerryapi',
      'type': 'static_library',
      'dependencies': [
        'jerryscript.gyp:jerrysource',
         '<(icu_gyp_path):icui18n',
         '<(icu_gyp_path):icuuc',
      ],
      'include_dirs': [
         'include',
         'jerryscript/jerry-core/include',
         'jerryscript/jerry-ext/include',
         'jerryscript/jerry-ext/common',
         'jerryscript/jerry-port/default/include',
         'v8jerry',
      ],
      'defines': [
        'JERRY_CPOINTER_32_BIT=1',
        'JERRY_GLOBAL_HEAP_SIZE=16384',
        'JERRY_LINE_INFO=1',
        'JERRY_DEBUGGER=0',
        'JERRY_ERROR_MESSAGES=1',
        'JERRY_V8_DUMP_BACKTRACE=1',
      ],
      'sources': [
        'api.cc',
        'api_ext.cc',
        'inspector.cc',
        'platform.cc',

        '<@(generated_sources)',

        'v8jerry/v8jerry_allocator.cpp',
        'v8jerry/v8jerry_allocator.hpp',
        'v8jerry/v8jerry_backing_store.cpp',
        'v8jerry/v8jerry_backing_store.hpp',
        'v8jerry/v8jerry_allocator.hpp',
        'v8jerry/v8jerry_callback.cpp',
        'v8jerry/v8jerry_callback.hpp',
        'v8jerry/v8jerry_flags.cpp',
        'v8jerry/v8jerry_flags.hpp',
        'v8jerry/v8jerry_handlescope.cpp',
        'v8jerry/v8jerry_handlescope.hpp',
        'v8jerry/v8jerry_isolate.cpp',
        'v8jerry/v8jerry_isolate.hpp',
        'v8jerry/v8jerry_templates.cpp',
        'v8jerry/v8jerry_templates.hpp',
        'v8jerry/v8jerry_utils.cpp',
        'v8jerry/v8jerry_utils.hpp',
        'v8jerry/v8jerry_value.cpp',
        'v8jerry/v8jerry_value.hpp',

        'jerryscript/jerry-ext/debugger/debugger-common.c',
        'jerryscript/jerry-ext/debugger/debugger-sha1.c',
        'jerryscript/jerry-ext/debugger/debugger-tcp.c',
        'jerryscript/jerry-ext/debugger/debugger-ws.c',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          'include',
        ],
      },
      'export_dependent_settings': [
        '<(icu_gyp_path):icui18n',
        '<(icu_gyp_path):icuuc',
      ],
    },
  ],
}
