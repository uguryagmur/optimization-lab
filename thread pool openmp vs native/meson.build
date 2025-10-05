project('monte carlo optimizations', 'cpp',
  version: '0.0.1',
  default_options: ['cpp_std=c++23', 'warning_level=3']
)

# Dependencies
# NOTE: these should match the [provide] section in your .wrap files
gtest_dep = dependency('gtest_main',
  fallback: ['gtest', 'gtest_main_dep']
)

benchmark_dep = dependency('benchmark',
  fallback: ['google-benchmark', 'benchmark_dep']
)

threads_dep = dependency('threads')

omp_dep = dependency('openmp')

# Build shared library: libThreadPool.so
monte_carlo_lib = shared_library('MonteCarlo',
  'src/MonteCarloPi.cpp',
  include_directories: include_directories('include'),
  dependencies: [threads_dep, omp_dep],
  install: true,
  version: meson.project_version(),
)

# Dependency wrapper for the library
monte_carlo_dep = declare_dependency(
  link_with: monte_carlo_lib,
  include_directories: include_directories('include'),
  dependencies: [threads_dep]
)

# Tests
test_exe = executable('runTest',
  'src/sanity_check.cpp',
  dependencies: [monte_carlo_dep, gtest_dep]
)
test('threadpool tests', test_exe)

# Benchmarks
bench_exe = executable('runBenchmark',
  'src/perf_comparison.cpp',
  dependencies: [monte_carlo_dep, benchmark_dep]
)
