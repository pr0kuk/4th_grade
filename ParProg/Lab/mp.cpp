#include "gtest/gtest.h"
#include <benchmark/benchmark.h>
#include <cmath>
#include <iostream>
#include <omp.h>
#include <random>
#include <vector>

constexpr int ISIZE = 3000;
constexpr int JSIZE = 3000;

#ifndef THREADS
#define THREADS 7
#endif

constexpr int max(int lhs, int rhs) { return lhs < rhs ? rhs : lhs; }

class Rand {
private:
  std::random_device dev;
  std::mt19937 rng;
  std::uniform_int_distribution<std::mt19937::result_type> dist;

public:
  Rand(unsigned long left = -10000, unsigned long right = 10000)
      : dev{}, rng{dev()}, dist{left, right} {}
  unsigned long get() { return dist(rng); }
  template <typename T = int>
  std::vector<std::vector<T>> getVector(int x, int y) {
    std::vector<std::vector<T>> data(x);
    for (auto &vec : data) {
      vec.resize(y);
      for (auto &elem : vec)
        elem = static_cast<T>(get());
    }
    return data;
  }
};

void sequential2A1(std::vector<std::vector<double>> &data) {
  for (int i = 0; i < ISIZE; ++i)
    for (int j = 0; j < JSIZE; ++j)
      data[i][j] = 10 * i + j;
}
void sequential2A2 (std::vector<std::vector<double>>& data) {
  for (int i = 0; i < ISIZE - 1; ++i)
    for (int j = 1; j < JSIZE; ++j)
      data[i][j] = std::sin(0.1 * data[i + 1][j - 1]);
}

void parallel2A1(std::vector<std::vector<double>> &data) {
  auto size = ISIZE / 8;
#pragma omp parallel for num_threads(THREADS)
  for (int k = 0; k < 8; ++k) {
    for (int i = k * size; i < (k + 1) * size; ++i)
      for (int j = 0; j < JSIZE; ++j)
        data[i][j] = 10 * i + j;
  }
}
void parallel2A2 (std::vector<std::vector<double>>& data) {
  for (int i = 0; i < ISIZE - 1; ++i) {
#pragma omp parallel for num_threads(THREADS)
    for (int j = 1; j < JSIZE; ++j) {
      data[i][j] = std::sin(0.1 * data[i + 1][j - 1]);
    }
  }
}

static void sequential2A1_bench(benchmark::State &state) {
  auto data = Rand{}.getVector<double>(ISIZE, JSIZE);
  for (auto _ : state) {
    auto copy = data;
    sequential2A1(copy);
  }
}
BENCHMARK(sequential2A1_bench);
static void sequential2A2_bench (benchmark::State& state)
{
  auto data = Rand {}.getVector<double> (ISIZE , JSIZE);
  for (auto _ : state)
  {
    auto copy = data;
    sequential2A2 (copy);
  }
}
BENCHMARK (sequential2A2_bench);
static void parallel2A1_bench(benchmark::State &state) {
  auto data = Rand{}.getVector<double>(ISIZE, JSIZE);
  for (auto _ : state) {
    auto copy = data;
    parallel2A1(copy);
  }
}
BENCHMARK(parallel2A1_bench);
static void parallel2A2_bench (benchmark::State& state)
{
  auto data = Rand {}.getVector<double> (ISIZE , JSIZE);
  for (auto _ : state)
  {
    auto copy = data;
    parallel2A2 (copy);
  }
}
BENCHMARK (parallel2A2_bench);

TEST(test, test1) {
  auto data = Rand{}.getVector<double>(ISIZE, JSIZE);
  auto copy = data;
  sequential2A1(data);
  sequential2A2 (data);
  parallel2A1(copy);
  parallel2A2 (copy);
  for (int i = 0; i < ISIZE; ++i)
    for (int j = 0; j < JSIZE; ++j)
      ASSERT_DOUBLE_EQ(data[i][j], copy[i][j]);
}

int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  int run_result = RUN_ALL_TESTS();

  ::benchmark::Initialize(&argc, argv);
  ::benchmark::RunSpecifiedBenchmarks();
  ::benchmark::Shutdown();

  return run_result;
}