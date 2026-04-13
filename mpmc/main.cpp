#include "log.h"
#include <thread>

const size_t test_count = 1000000;

void test_func_1(void)
{
   for(size_t i = 0; i < test_count; i++)
   {
     LOG_INFO("This is %s and the count is %lu", __FUNCTION__, i);
   }
}

void test_func_2(void)
{
   for(size_t i = 0; i < test_count; i++)
   {
     LOG_INFO("This is %s and the count is %lu", __FUNCTION__, i);
   }
}


int main(void)
{
  Log::instance()->init(LogLevel::INFO);
  std::thread t1(test_func_1);
  std::thread t2(test_func_2);
  t1.join();
  t2.join();


  Log::instance()->log_close();
  return 0;
}
