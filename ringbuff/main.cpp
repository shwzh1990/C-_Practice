#include "ringbuff.h"
#include <iostream>
int main(void)
{
  
  ringbuff<std::string> ringbuf;
  ringbuf.put("hello");
  ringbuf.put("good");
  ringbuf.put(std::string ("this is a book"));
  const char* test_str = "fjaksifkslfklsajfklsafksdsafs";
  ringbuf.put(test_str);

  std::string s = "";
  while(ringbuf.pop(s))
  {
     std::cout << s << "\n";
  }

  return 0;
}
