{
  'variables': {
  },
  'targets': [
    {
      'target_name': 'logStream',
      'sources': [ 
        'src/logStream.cc'
      ],
      'include_dirs+': [
      ],
      'cflags!': [ '-fno-exceptions' ],
      'cflags_cc!': [ '-fno-exceptions' ],
      'conditions': [
        ['OS=="mac"', {
          'libraries': [
            '-lc++ -lc++abi'
          ],         
          'xcode_settings': {
            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
            'OTHER_CFLAGS': [ '-g', '-mmacosx-version-min=10.7', '-std=c++11', '-stdlib=libc++', '-O3', '-D__STDC_CONSTANT_MACROS', '-D_FILE_OFFSET_BITS=64', '-D_LARGEFILE_SOURCE', '-Wall' ],
            'OTHER_CPLUSPLUSFLAGS': [ '-g', '-mmacosx-version-min=10.7', '-std=c++11', '-stdlib=libc++', '-O3', '-D__STDC_CONSTANT_MACROS', '-D_FILE_OFFSET_BITS=64', '-D_LARGEFILE_SOURCE', '-Wall' ]

          }
        }]
      ],         
      'conditions': [
        ['OS=="win"', {
          'libraries': [
          ],
          'include_dirs': [
          ],
          'defines': [
            'snprintf=_snprintf',
            '_USE_MATH_DEFINES'  # for M_PI
          ]
        }, 
        {  # 'OS!="win"'
          'libraries': [
          ],
        }]
      ]
    }
  ]
}
