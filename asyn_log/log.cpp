#include "log.h"
#include <bits/floatn-common.h>
#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <thread>
#include <stdarg.h>
#include <exception>

Log::Log():rb(1024)
{
  aysn_log_ = nullptr;
  enable_asyn_ = false;
  level_ = LogLevel::INFO;
  
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
  std::string str = "";
  size_t attemp = 0;
  if(level <= level_)
  {
     str = append_level(level);
     va_start(args1, fmt);
     va_copy(args2, args1);
     int size = std::vsnprintf(nullptr, 0, fmt, args1);
     va_end(args1);

     if(size  < 0)
     {
       va_end(args2);
       throw std::runtime_error("fmt error");
     }

     std::vector<char> buff(size + 1);
     vsnprintf(buff.data(), buff.size(), fmt, args2);
     va_end(args2);
    if(enable_asyn_)
    {
      while(!rb.put(str + std::string(buff.data(), size)))
      {
        attemp++;
        if(attemp <= 100)
        {
          __builtin_ia32_pause();
        }
        else if(attemp <= 500)
        {
          std::this_thread::yield();
        }
        else
        {
          std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
      }
    }
    else
    {
       std::cout << str << "\n";
    }
     

  }
}

std::string Log::append_level(LogLevel level)
{
  std::string str = "";
  switch(level)
  {
    case LogLevel::INFO:
      str = "[INFO]: ";
    break;

    case LogLevel::DEBUG:
      str = "[DEBUG]: ";
    break;

    case LogLevel::WARN:
      str = "[WARN]: ";
    break;

    case LogLevel::ERROR:
      str = "[ERROR]: ";
    break;
    default:
      str = "[INFO]: ";
    break;
  }
  return str;
}

Log* Log::instance(void)
{
   static Log log;
   return &log;
}

void Log::asyn_log_func(void)
{
  std::string s = "";
  while(Log::instance()->rb.pop(s))
  {
     std::cout << s << "\n";
  }
  std::this_thread::yield();
}
