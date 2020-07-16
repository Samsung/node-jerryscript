{
  'variables': {
    'deps_path%': '..'
  },
  'targets': [
    {
      'target_name': 'glib',
      'type': '<(library)',
      'dependencies': [ '<(deps_path)/uv/uv.gyp:libuv' ],
      'cflags': [
        '<!@(pkg-config --cflags glib-2.0)',
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
          '<!@(pkg-config --cflags glib-2.0)',
        ],
        'libraries': [
          '<!@(pkg-config --libs glib-2.0)',
        ],
        'include_dirs': [
          'src',
        ],
      },
    }
  ],
}
