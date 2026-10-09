project('SMECT', 'cpp', default_options: ['cpp_std=c++23'])

curl_dep = dependency('libcurl')
crypto = dependency('libcrypto')

executable('smect',
  'src/main.cpp',
  'src/download.cpp',
  'src/recipe.cpp',
  'src/sha256.cpp',
  dependencies: [curl_dep, crypto],
)
