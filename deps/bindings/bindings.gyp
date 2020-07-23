{
  'variables': {
    'deps_path%': '..',
    'host_platform%': 'none',
    'platform_libs%': '',
    'conditions': [
      ['host_platform=="tizen"', {
        'platform_libs': 'dlog'
      }],
    ],
  },
  'targets': [
    {
      'target_name': 'glib',
      'type': '<(library)',
      'dependencies': [ '<(deps_path)/uv/uv.gyp:libuv' ],
      'cflags': [
        '<!@(pkg-config --cflags glib-2.0 <(platform_libs))',
      ],
      'include_dirs': [
        'src',
      ],
      'sources': [
        'src/node_bindings.cc',
      ],
      'direct_dependent_settings': {
        'defines': [
          'ENABLE_NODE_BINDINGS'
        ],
        'cflags': [
          '<!@(pkg-config --cflags glib-2.0 <(platform_libs))',
        ],
        'libraries': [
          '<!@(pkg-config --libs glib-2.0 <(platform_libs))',
        ],
        'include_dirs': [
          'src',
        ],
      },
    }
  ],
}
