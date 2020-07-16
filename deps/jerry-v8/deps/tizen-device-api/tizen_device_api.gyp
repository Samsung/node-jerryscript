{
   'variables': {
   },

  'targets': [
    {
      'target_name': 'device_api',
      'type': 'static_library',
      'dependencies': [ ],
      'include_dirs': [
          'src',
          '../v8-headers',
      ],
      'direct_dependent_settings': {
      },
      'defines': [
      ],
      'sources': [
        'src/Extension.cpp',
        'src/ExtensionAdapter.cpp',
        'src/ExtensionManager.cpp',
        'src/TizenDeviceAPILoader.cpp',
      ],
      'cflags': [
      ],
      'conditions': [
      ]
    },
  ],
}
