#include "ringbuff.h"
#include <thread>
enum class LogLevel
{
	 INFO,
	 DEBUG,
	 WARN,
	 ERROR
 };  



class Log
{
	public: 
               Log();
	       ~Log();
	       void init(LogLevel level, bool enable_asyn = true);
				 static void asyn_log_func(void);
				 void write(LogLevel level, const char* fmt, ...);
				 static Log* instance();
		void log_close();
  private:
				 std::unique_ptr<std::thread> aysn_log_;
				 LogLevel level_;
				 bool enable_asyn_;
				 ringbuff<std::string> rb;
				 int append_level(LogLevel level, char* buff);
				 bool close_;
};

#define LOG_BASE(level, fmt, ...) \
	do\
  {  Log* log = Log::instance(); \
    log->write(level, fmt, ##__VA_ARGS__); \
	}while(0);\

#define LOG_INFO(fmt, ...) do{LOG_BASE(LogLevel::INFO, fmt, ##__VA_ARGS__)}while(0)
#define LOG_DEBUG(fmt, ...) do{LOG_BASE(LogLevel::DEBUG, fmt, ##__VA_ARGS__)}while(0)
#define LOG_ERROR(fmt, ...) do{LOG_BASE(LogLevel::ERROR, fmt, ##__VA_ARGS__)}while(0)
#define LOG_WARN(fmt, ...) do{LOG_BASE(LogLevel::WARN, fmt, ##__VA_ARGS__)}while(0)
