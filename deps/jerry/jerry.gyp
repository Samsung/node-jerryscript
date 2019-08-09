{
  'variables': {
  },
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
        'include_dirs': [ '.' ]
      },
      'sources': [
        'jerryscript.c',
        'jerryscript-port-default.c',
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
