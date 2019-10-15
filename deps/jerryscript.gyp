{
  'targets': [
    {
      'target_name': 'jerry',
      'type': 'static_library',
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

      'variables': {
        'generated_sources': [
          '<(SHARED_INTERMEDIATE_DIR)/jerry/jerryscript.c',
          '<(SHARED_INTERMEDIATE_DIR)/jerry/jerryscript-config.h',
          '<(SHARED_INTERMEDIATE_DIR)/jerry/jerryscript.h',
          '<(SHARED_INTERMEDIATE_DIR)/jerry/jerryscript-port-default.c',
          '<(SHARED_INTERMEDIATE_DIR)/jerry/jerryscript-port-default.h',
        ]
      },
      'defines': [
        'JERRY_GLOBAL_HEAP_SIZE=10*1024',
        'JERRY_ERROR_MESSAGES=1',
        'JERRY_LINE_INFO=1', 
        'JERRY_SYSTEM_ALLOCATOR=1',
        'JERRY_CPOINTER_32_BIT=1'
      ],

      'actions': [
        {
          'action_name': 'Prepare JerryScript',
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
      'sources': [
        '<@(generated_sources)',        
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
