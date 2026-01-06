#include "log.h"
#include <chrono>

const size_t ops_counter = 10000000;

int main(void)
{
  Log::instance()->init(LogLevel::INFO);
  size_t start = std::chrono::high_resolution_clock::now();
  for(size_t i = 0; i < ops_counter; ++i)
  {
    LOG_INFO("The count of %d\n", i);
  }
  size_t end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> duration = end - start;
  double ops_per_sec = ops_counter / duration.count();
  std::cout << "finish " << ops_counter << "th operation\n";
  std::cout << "total time consuming: " << duration.count() << "s\n";
  std::cout << ops_per_sec / 1e6 << "millions/s\n";  




  return 0;
}
