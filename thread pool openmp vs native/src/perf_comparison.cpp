#include <benchmark/benchmark.h>
#include "MonteCarloPi.h"


static void BM_ThreadPoolMonteCarlo(benchmark::State& state) {
    for (auto _ : state) {
      std::size_t numSamples = state.range(0);
      benchmark::DoNotOptimize(getMonteCarloPi_ThreadPool(numSamples, 42));
    }
}
BENCHMARK(BM_ThreadPoolMonteCarlo)->RangeMultiplier(10)->Range(10, 10'000'000);

static void BM_OpenMPMonteCarlo(benchmark::State& state) {
    for (auto _ : state) {
      std::size_t numSamples = state.range(0);
      benchmark::DoNotOptimize(getMonteCarloPi_ThreadPool(numSamples, 42));
    }
}
BENCHMARK(BM_OpenMPMonteCarlo)->RangeMultiplier(10)->Range(10, 10'000'000);

BENCHMARK_MAIN();
