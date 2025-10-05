#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include <random>
#include <omp.h>

#include "MonteCarloPi.h"


class ThreadPool
{
  public:
    ThreadPool(std::size_t numThreads);
    ~ThreadPool();
    
    template <typename F, typename... Args>
    auto enqueueTask(F&& func, Args&&... args) -> std::future<std::result_of_t<F(Args...)>>
    {
      using ReturnType = std::result_of_t<F(Args...)>;

      auto taskPtr = std::make_shared<std::packaged_task<ReturnType()>>(
          std::bind(std::forward<F>(func), std::forward<Args>(args)...)
      );

      std::future<ReturnType> future = taskPtr->get_future();
      
      // critical region
      {
        std::unique_lock<std::mutex> lock(_mutex);
        _taskQueue.emplace([taskPtr]() { (*taskPtr)(); });
      }

      _condition.notify_one();
      return future;
    }

    std::size_t numPendingTasks() const;
    std::size_t numThreads() const;

  private:

    void threadRunner();

    std::queue<std::function<void()>> _taskQueue;
    std::condition_variable _condition;
    std::vector<std::thread> _threads;
    std::atomic<bool> _stop;

    mutable std::mutex _mutex;
};

ThreadPool::ThreadPool(std::size_t numThreads): _stop(false)
  {
    _threads.reserve(numThreads);
    auto workerFunc = std::bind(&ThreadPool::threadRunner, this);

    for (std::size_t i=0; i<numThreads; ++i)
    {
      _threads.emplace_back(workerFunc);
    }
    
  }

ThreadPool::~ThreadPool()
{
  // critical region
  {
    std::unique_lock<std::mutex> lock(_mutex);
    _stop = true;
  }

  // notify all threads for return
  _condition.notify_all();

  for (auto& worker: _threads)
  {
    if (worker.joinable())
    {
      worker.join();
    }
  }
}

std::size_t ThreadPool::numPendingTasks() const
{
  std::unique_lock<std::mutex> lock(_mutex);
  return _taskQueue.size();
}

std::size_t ThreadPool::numThreads() const
{
  return _threads.size();
}


void ThreadPool::threadRunner()
{
  while (true)
  {
    std::function<void()> task;

    // critical region for retrieving a task
    {
      std::unique_lock<std::mutex> lock(_mutex);

      _condition.wait(lock, [this]{return _stop || !_taskQueue.empty(); });

      if (_stop && _taskQueue.empty())
      {
        return;
      }

       task = std::move(_taskQueue.front());
       _taskQueue.pop();
    }

    task();
  }
}

double getMonteCarloPi_ThreadPool(std::size_t numSamples, std::size_t seed)
{
  std::atomic<std::size_t> totalPoints = 0;
  std::atomic<std::size_t> totalInPoints = 0;
  std::size_t numThreads = std::thread::hardware_concurrency();

  auto runner = [&]() -> void
  { 
    thread_local std::mt19937_64 gen(std::random_device{}());
    gen.seed(seed);

    thread_local std::uniform_real_distribution<> dist(-1.0, 1.0);
    thread_local std::size_t numInPoints = 0;
    std::size_t samplePerThread = numSamples / numThreads;

#pragma omp simd
    for (std::size_t i=0; i < samplePerThread; ++i)
    {
      double x = dist(gen); 
      double y = dist(gen); 
      if (x*x + y*y <= 1.0)
      {
        ++numInPoints;
      }
    }

    totalPoints += samplePerThread;
    totalInPoints += numInPoints;
  };

  ThreadPool tp(numThreads);
  std::vector<std::future<void>> futures(numThreads);

  for (std::size_t i=0; i<numThreads; ++i)
  {
    futures[i] = tp.enqueueTask(runner);
  }

  // wait for threads to be completed
  for (auto& f: futures)
  {
    f.get();
  }

  return 4.0 * totalInPoints / totalPoints;
}


double getMonteCarloPi_OpenMP(std::size_t numSamples, std::size_t seed)
{
  std::atomic<std::size_t> totalPoints = 0;
  std::atomic<std::size_t> totalInPoints = 0;
  unsigned int numThreads = std::thread::hardware_concurrency();
  omp_set_num_threads(numThreads);

#pragma omp parallel
  {
    thread_local std::mt19937_64 gen(std::random_device{}());
    gen.seed(seed);

    thread_local std::uniform_real_distribution<> dist(-1.0, 1.0);
    thread_local std::size_t numInPoints = 0;

    std::size_t samplePerThread = numSamples / numThreads;

#pragma omp simd
    for (std::size_t i=0; i < samplePerThread; ++i)
    {
      double x = dist(gen); 
      double y = dist(gen); 
      if (x*x + y*y <= 1.0)
      {
        ++numInPoints;
      }
    }

    totalPoints += samplePerThread;
    totalInPoints += numInPoints;
  }

  return 4.0 * totalInPoints / totalPoints;
}
