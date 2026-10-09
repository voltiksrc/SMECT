project('SMECT', 'cpp', default_options: ['cpp_std=c++23'])

curl_dep = dependency('libcurl')

executable('smect',
  'src/main.cpp',
  'src/download.cpp',
  'src/recipe.cpp',
  dependencies: [curl_dep],
)
