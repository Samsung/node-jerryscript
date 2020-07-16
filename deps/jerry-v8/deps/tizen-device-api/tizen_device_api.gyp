{
  'variables': {
    'deps_path%': '..'
  },
  'targets': [
    {
      'target_name': 'device_api',
      'type': 'static_library',
      'include_dirs': [
        'src',
        '<(deps_path)/v8-headers',
      ],
      'cflags': [
        '-Werror',
        '<!@(pkg-config --cflags dlog)',
      ],
      'direct_dependent_settings': {
        'cflags': [
          '<!@(pkg-config --cflags dlog)',
        ],
        'libraries': [
          '<!@(pkg-config --libs dlog)',
        ],
      },
      'defines': [
      ],
      'sources': [
        'src/Extension.cpp',
        'src/ExtensionAdapter.cpp',
        'src/ExtensionManager.cpp',
        'src/TizenDeviceAPILoader.cpp',
      ],
    },
  ],
}
