{
  'targets': [
    {
      'target_name': 'jerrylib',
      'type': 'static_library',
      'actions': [
        {
          'action_name': 'jerrylib',
          'inputs': [
            'jerryscript/tools/build.py',
          ],
          'outputs': [
            'jerryscript/build/lib/libjerry-core.a',
          ],
          'action': [
            'python',
            'jerryscript/tools/build.py',
            '--debug',
            '--error-messages=on',
            '--line-info=on',
          ],
        },
      ],
    },
    {
      'target_name': 'jerryapi',
      'type': 'static_library',
      'dependencies': [
#         'jerryscript.gyp:jerrylib',
          '<(icu_gyp_path):icui18n',
          '<(icu_gyp_path):icuuc',
      ],
      'include_dirs': [
         'include',
         'jerryscript/jerry-core/include',
      ],
      'sources': [
        'api.cc',
        'api_ext.cc',
        'inspector.cc',
        'platform.cc',
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
