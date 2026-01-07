#include "log.h"


int main(void)
{
  Log::instance()->init(LogLevel::INFO);
  
  LOG_INFO("hello world!!!");
  Log::instance()->log_close();
  return 0;
}
