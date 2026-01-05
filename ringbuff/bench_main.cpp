#include "ringbuff.h"
#include <cstddef>
#include <iostream>
#include <thread>
#include <chrono>


const size_t TOTAL_OPS = 10000000;

void run_bench()
{
   ringbuff<size_t> rb(1024);
   auto start = std::chrono::high_resolution_clock::now();

   std::thread consumer([&]()
   {
     size_t data;
     for(size_t i = 0; i < TOTAL_OPS; i++)
     {
       while(!rb.pop(data))
       {
         __builtin_ia32_pause();
       }

     }
   });

   std::thread producer([&](){
       for(size_t i = 0; i < TOTAL_OPS; ++i)
       {
          while(!rb.put(std::move(i)))
          {
            __builtin_ia32_pause();
          }
       }


       });
   producer.join();
   consumer.join();
   auto end = std::chrono::high_resolution_clock::now();
   std::chrono::duration<double> duration = end - start;
   double ops_per_sec = TOTAL_OPS / duration.count();
   std::cout << "finish " << TOTAL_OPS << "th operation\n";
   std::cout << "total time consuming: " << duration.count() << "s\n";
   std::cout << ops_per_sec / 1e6 << "millions/s\n";  

}

int main()
{
  run_bench();
  return 0;
}
