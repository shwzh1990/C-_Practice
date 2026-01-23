#include "log.h"
#include "threadpool.h"
#include <chrono>
#include <future>
#include <thread>
#include "stealqueue.h"



const size_t cal_times = 100000000;
std::promise<size_t> result_one_p;
std::promise<size_t> result_two_p;
std::promise<size_t> result_three_p;
std::future<size_t> result_one = result_one_p.get_future();
std::future<size_t> result_two = result_two_p.get_future();
std::future<size_t> result_three = result_three_p.get_future();

void task_one(void)
{
   size_t sum_t;
   for(size_t i = 1; i < cal_times; ++i)
   {
       sum_t =  i;
   }
   LOG_INFO("for task one the result is %lu", sum_t);
   result_one_p.set_value(sum_t);
}

void task_two(void)
{
   size_t sum_t = 1;
   for(size_t i = 1; i < cal_times; ++i)
   {
       sum_t += i;
   }
   LOG_INFO("For task two the result is %lu", sum_t);
   result_two_p.set_value(sum_t);
}

void task_three(void)
{
   size_t sum_t = 1;
   for(size_t i = 1; i < cal_times; ++i)
   {
       sum_t += i;
   }
   LOG_INFO("For task three the result is %lu", sum_t);
   result_three_p.set_value(sum_t);
}


int main(void)
{
  Log::instance()->init(LogLevel::INFO);
  WorkStealingPool pool(4);
  for(size_t i = 0; i < 20; i++)
  {
    pool.submit(0, [i]{
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
 
          });
  }
  std::this_thread::sleep_for(std::chrono::seconds(2));
  Log::instance()->log_close();
  return 0;
}
