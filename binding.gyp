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
