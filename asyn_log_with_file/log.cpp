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
#include <filesystem>
#include <fcntl.h>    // 定义了 O_CREAT, O_RDWR, O_APPEND 等标志位
#include <sys/types.h> // 定义了 mode_t (用于文件权限)
#include <sys/stat.h>  // 定义了权限常量 (如 S_IRUSR)
#include <unistd.h>    // 定义了 close() 系统调用
namespace fs = std::filesystem;

Log::Log():rb(1024)
{
  aysn_log_ = nullptr;
  enable_asyn_ = false;
  level_ = LogLevel::INFO;
  close_ = false;
  log_fd_ = -1; 
  file_path_ = "";
  file_name_ = "";
  file_cache_size.reserve(32*1024);
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
  if(log_fd_)
  {
    close(log_fd_);
  }
}

void Log::init(LogLevel level, bool enable_asyn, std::string_view file_path, std::string_view file_name)
{
   level_ = level;
   enable_asyn_ = enable_asyn;
   file_path_ = file_path ;
   file_name_ = file_name;
   file_name_ = file_path_ + file_name_;
   if(enable_asyn_)
   {
    aysn_log_ = std::make_unique<std::thread>(asyn_log_func);
   }
   if(fs::exists(file_name_))
   {
     log_fd_= open(file_name_.c_str(), O_WRONLY |  O_APPEND, 0644);
     if(log_fd_ < 0)
     {
      perror("cannot open the file....\n");
      throw std::runtime_error("cannot open the file");
     }
   }
   else
   {
     log_fd_= open(file_name_.c_str(), O_WRONLY |  O_APPEND | O_CREAT, 0644);
   }
}

void Log::write_log(LogLevel level, const char* fmt,...)
{
  va_list args1;
  va_list args2;
  size_t attemp = 0;
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
        temp_buff[size + debug_suffix_size] = '\n';
        temp_buff[size + debug_suffix_size + 1] = '\0';
     }
     else
     {
      std::vector<char> buff(size + 2);
      vsnprintf(buff.data(), buff.size(), fmt, args2);
      buff[size + 1] = '\n';
      buff[size + 2] = '\0';
     }
     va_end(args2);
    if(enable_asyn_)
    {
      while(!rb.put(temp_buff))
      {
        attemp++;
        if(attemp <= 200)
        {
          __builtin_ia32_pause();
        }
        else 
        {
          std::this_thread::yield();
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
  if(aysn_log_ && aysn_log_->joinable()) 
  {
    aysn_log_->join();
  }
}

void Log::asyn_log_func(void)
{
  auto last_flush_time = std::chrono::steady_clock::now();
  ssize_t result = 0;
  while(1)
  {
    std::string temp_str;
    bool has_data = Log::instance()->rb.pop(temp_str);
    if(has_data)
    {
      Log::instance()->file_cache_size.append(temp_str);
    }
    else
    {
      __builtin_ia32_pause();
    }
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_flush_time);
    if((!Log::instance()->file_cache_size.empty() && (Log::instance()->file_cache_size.size() >= 2 * 1024)) || duration.count() >= 10)
    {
       result = write(Log::instance()->log_fd_, Log::instance()->file_cache_size.data(), Log::instance()->file_cache_size.size());
      if(result != static_cast<ssize_t>(Log::instance()->file_cache_size.size()))
      {
        fprintf(stderr, "the read write in num is %ld, should be write in bytes is %ld", result, Log::instance()->file_cache_size.size());
      }
      Log::instance()->file_cache_size.clear();
      last_flush_time = now;
    }
    
    if(Log::instance()->close_ == true) 
    {
      while(Log::instance()->rb.pop(temp_str))
      {
        Log::instance()->file_cache_size.append(temp_str);
      }
      result = write(Log::instance()->log_fd_, Log::instance()->file_cache_size.data(), Log::instance()->file_cache_size.size());
      (void)result;
      Log::instance()->file_cache_size.clear();
      return;
    }
  }
}
