#include "log.h"


int main(void)
{
  Log::instance()->init(LogLevel::INFO);
  
  LOG_INFO("hello world!!!");

  return 0;
}
