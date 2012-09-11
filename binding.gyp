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
          'xcode_settings': {
            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES'
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
