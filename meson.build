project('SMECT', 'cpp', default_options: ['cpp_std=c++23'])

curl_dep = dependency('libcurl')
crypto = dependency('libcrypto')
archive_dep = dependency('libarchive')

executable('smect',
  'src/main.cpp',
  'src/download.cpp',
  'src/recipe.cpp',
  'src/sha256.cpp',
  'src/extract.cpp',
  dependencies: [curl_dep, crypto, archive_dep],
  install: true,
)
