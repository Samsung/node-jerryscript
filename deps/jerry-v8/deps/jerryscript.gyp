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
        'JERRY_ERROR_MESSAGES=1',
        'JERRY_LINE_INFO=1', 
        'JERRY_CPOINTER_32_BIT=1',
        'JERRY_VM_EXEC_STOP=1',
      ],

      'sources': [
        '<@(generated_sources)',
      ],
      'cflags': [
        '-std=c99',
        '-pedantic',
      ],
      'conditions': [
        ['target_arch=="x32" or target_arch=="x64"', {
          'cflags': [
            '-mfpmath=sse',
            '-msse2',
          ]
        }],
        ['target_arch=="x64"', {
          'defines': [
            'JERRY_SYSTEM_ALLOCATOR=0',
            'JERRY_GLOBAL_HEAP_SIZE=(200*1024)',
          ]
        }, {
          'defines': [
            'JERRY_SYSTEM_ALLOCATOR=1'
          ]
        }],

        ['want_separate_host_toolset==1', {
          'toolsets': ['host', 'target'],
        }, {
          'toolsets': ['target'],
        }],
      ]
    },
  ],
}
