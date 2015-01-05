{
  'conditions': [
    ['OS=="win"', {
      'variables': {
        'MAGICK_ROOT%': '<!(python winmagick.py)',
        # download the dll binary and check off for libraries and includes
        'OSX_VER%': "0",
      }
    }],
    ['OS=="mac"', {
      'variables': {
        # matches 10.9.X , 10.10 and outputs 10.9 , 10.10
        'OSX_VER%': "<!(sw_vers | grep 'ProductVersion:' | grep -o '10.[0-9]*')",
      }
    }, {
      'variables': {
        'OSX_VER%': "0",
      }
    }]
  ],
  'targets': [
    {
      'target_name': 'magicklib',
      'cflags!': [ '-fno-exceptions' ],
      'cflags_cc!': [ '-fno-exceptions' ],
      'sources': [
        'src/color.cc',
        'src/job.cc',
        'src/imageprocessor.cc',
        'src/image.cc',
        'src/nodemagick.cc'
      ],
      'include_dirs': [
        "<!(node -e \"require('nan')\")",
        'src'
      ],
      'defines': [
        # 'NODEMAGICK_USE_STL_MAP' # uncomment to use std::map instead of std::vector in places
      ],
      'configurations': {
        'Release': {
          'msvs_settings': {
            'VCCLCompilerTool': {
              'RuntimeLibrary': 2 # dynamic release, linking statically crashes when trying to delete allocated image, access image comment, etc.
            }
          }
        }
      },
      'conditions': [
        ['OS=="win"', {
          'msvs_settings': {
            'VCCLCompilerTool': {
              'ExceptionHandling': 1,
              'AdditionalOptions': [
                '/EHsc' # ExceptionHandling=1 is not enough
              ]
            }
          },
          'libraries': [
            '-l<(MAGICK_ROOT)/lib/CORE_RL_magick_.lib',
            '-l<(MAGICK_ROOT)/lib/CORE_RL_Magick++_.lib',
            '-l<(MAGICK_ROOT)/lib/CORE_RL_wand_.lib',
            '-l<(MAGICK_ROOT)/lib/X11.lib'
          ],
          'include_dirs': [
            '<(MAGICK_ROOT)/include',
          ]
        }],
        ['OS=="win" and target_arch!="x64"', {
          'defines': [
            '_SSIZE_T_',
          ]
        }],
        ['OSX_VER == "10.9" or OSX_VER == "10.10"', {
          'xcode_settings': {
            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
            'OTHER_CFLAGS': [
              '<!@(Magick++-config --cflags)'
            ],
            'OTHER_CPLUSPLUSFLAGS' : [
              '<!@(Magick++-config --cflags)',
              '-std=c++11',
              '-stdlib=libc++',
            ],
            'OTHER_LDFLAGS': ['-stdlib=libc++'],
            'MACOSX_DEPLOYMENT_TARGET': '10.7', # -mmacosx-version-min=10.7
          },
          'libraries': [
             '<!@(Magick++-config --ldflags --libs)',
          ],
          'cflags': [
            '<!@(Magick++-config --cflags --cppflags)'
          ],
        }],
        ['OS=="mac"', {
          'xcode_settings': {
            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
            'OTHER_CFLAGS': [
              '<!@(Magick++-config --cflags)'
            ]
          },
          'libraries': [
            '<!@(Magick++-config --ldflags --libs)'
          ],
          'cflags': [
            '<!@(Magick++-config --cflags --cppflags)'
          ],
        }],
        ['OS=="linux"', { # not windows not mac
          'libraries': [
            '<!@(Magick++-config --ldflags --libs)',
          ],
          'cflags': [
            '<!@(Magick++-config --cflags --cppflags)'
          ],
        }]
      ]
    }
  ]
}
