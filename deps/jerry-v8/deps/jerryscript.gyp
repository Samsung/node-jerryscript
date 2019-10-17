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
      'target_name': 'jerry',
      'type': 'static_library',
      'dependencies': [ 'jerrysource' ],
      'include_dirs+': [
        '.',
        '<(DEPTH)',
        '<(SHARED_INTERMEDIATE_DIR)'
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '.',
          '<(SHARED_INTERMEDIATE_DIR)/jerry/',
        ]
      },
      'defines': [
        'JERRY_GLOBAL_HEAP_SIZE=10*1024',
        'JERRY_ERROR_MESSAGES=1',
        'JERRY_LINE_INFO=1', 
        'JERRY_SYSTEM_ALLOCATOR=1',
        'JERRY_CPOINTER_32_BIT=1'
      ],

      'sources': [
        '<@(generated_sources)',
      ],
      'cflags': [
        '-std=c99',
        '-pedantic',
        '-mfpmath=sse',
        '-msse2',
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
