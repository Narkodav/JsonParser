project(
    'JsonParser',
    'cpp',
    version: '1.0.0',
    default_options: [
        'cpp_std=c++20',
        'warning_level=3',
    ],
    meson_version: '>=0.64.0',
)

cpp = meson.get_compiler('cpp')

# SIMD detection (optional)
simd_args = []
if cpp.has_argument('-mavx2')
    simd_args += '-mavx'
endif

jsonparser_dep = declare_dependency(
    include_directories: include_directories('include'),
    compile_args: simd_args,
)
meson.override_dependency('jsonparser', jsonparser_dep)

# Installation
install_subdir('include', install_dir: get_option('includedir'), strip_directory: true)

pkg = import('pkgconfig')
pkg.generate(
    name: 'JsonParser',
    description: 'Header-only JSON parser',
    version: meson.project_version(),
    extra_cflags: simd_args
)

# Benchmarks / examples
if (get_option('build_benchmarks') or get_option('build_examples')) and not meson.is_subproject()
    subdir('tests')
endif

if get_option('build_benchmarks') and not meson.is_subproject()
    executable(
        'JsonParserBenchmark',
        'tests/benchmark.cpp',
        dependencies: jsonparser_dep,
    )
endif

if get_option('build_examples') and not meson.is_subproject()
    executable(
        'JsonParserExamples',
        'tests/example.cpp',
        dependencies: jsonparser_dep,
    )
endif