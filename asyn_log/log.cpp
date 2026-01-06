#include "log.h"
#include <bits/floatn-common.h>
#include <chrono>
#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <iostream>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <thread>
#include <stdarg.h>
#include <exception>

Log::Log():rb(1024)
{
  aysn_log_ = nullptr;
  enable_asyn_ = false;
  level_ = LogLevel::INFO;
  close_ = false;
  
}

Log::~Log()
{
  if(enable_asyn_)
  {
    if((aysn_log_ != nullptr) && (aysn_log_->joinable()))
    {
      aysn_log_->join();
    }
  }
}

void Log::init(LogLevel level, bool enable_asyn)
{
   level_ = level;
   enable_asyn_ = enable_asyn;
   if(enable_asyn_)
   {
    aysn_log_ = std::make_unique<std::thread>(asyn_log_func);
   }
}

void Log::write(LogLevel level, const char* fmt,...)
{
  va_list args1;
  va_list args2;
  //size_t attemp = 0;
  size_t debug_suffix_size = 0;
  char temp_buff[1024] = {0};
  if(level <= level_)
  {
     debug_suffix_size = (size_t)append_level(level, temp_buff);
     va_start(args1, fmt);
     va_copy(args2, args1);
     int size = std::vsnprintf(nullptr, 0, fmt, args1);
     va_end(args1);

     if(size  < 0)
     {
       va_end(args2);
       throw std::runtime_error("fmt error");
     }

     if(size < 1024)
     {
        vsnprintf(temp_buff + debug_suffix_size , sizeof(temp_buff) - debug_suffix_size - 1, fmt, args2);
     }
     else
     {
      std::vector<char> buff(size + 1);
      vsnprintf(buff.data(), buff.size(), fmt, args2);
     }
     va_end(args2);
    if(enable_asyn_)
    {
      while(!rb.put(temp_buff))
      {
        //attemp++;
        //if(attemp <= 200)
        {
          __builtin_ia32_pause();
        }
       // else 
        {
         // std::this_thread::yield();
        }
        
      }
    }
    else
    {
       std::cout << temp_buff << "\n";
    }
     

  }
}

int Log::append_level(LogLevel level, char* buff)
{
  int size = 0;
  switch(level)
  {
    case LogLevel::INFO:
      size = std::sprintf(buff, "[INFO]: ");
    break;

    case LogLevel::DEBUG:
       size = std::sprintf(buff, "[DEBUG]: ");
    break;

    case LogLevel::WARN:
       size = std::sprintf(buff, "[WARN]: ");
    break;

    case LogLevel::ERROR:
       size = std::sprintf(buff, "[ERROR]: ");
    break;
    default:
      size = std::sprintf(buff, "[INFO]: ");
    break;
  }
  return size;
}

Log* Log::instance(void)
{
   static Log log;
   return &log;
}

void Log::log_close(void)
{
	close_ = true;
  std::cout << std::flush;
}

void Log::asyn_log_func(void)
{
  std::string s = "";
  while(1)
  {
    while(Log::instance()->rb.pop(s))
    {
       std::cout << s << "\n";
    }
    //std::this_thread::yield();
    if(Log::instance()->close_ == true) return;
  }
}
