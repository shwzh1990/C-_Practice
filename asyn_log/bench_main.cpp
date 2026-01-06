#include "log.h"
#include <chrono>

const size_t ops_counter = 1000000;

int main(void)
{
  Log::instance()->init(LogLevel::INFO);
  auto start = std::chrono::high_resolution_clock::now();
  for(size_t i = 0; i < ops_counter; ++i)
  {
    LOG_INFO("The count of %d\n", i);
  }
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> duration = end - start;
  double ops_per_sec = ops_counter / duration.count();

  LOG_INFO("finish %zu operations\n", ops_counter);
  LOG_INFO("total time consuming: %f seconds\n", duration.count());
  LOG_INFO("throughput: %f millions/s\n", ops_per_sec / 1e6);
  Log::instance()->log_close();
// 将最后的统计信息改为 std::cerr
std::cerr << "finish " << ops_counter << " operations\n";
std::cerr << "total time consuming: " << duration.count() << " seconds\n";
std::cerr << "throughput: " << ops_per_sec / 1e6 << " millions/s\n";
return 0;
}
