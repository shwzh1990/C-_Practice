CXX = g++
CXXFLAGS = -std=c++20 -O3 -Wall -Wextra 
CXXDEBUG = -std=c++20 -O1 -fsanitize=address -Wall -Wextra
# 目标可执行文件
TARGET = threadpool_steal.o

TEST_TARGET = threadpool_steal_bench_test.o
# 源文件
SRCS = main.cpp log.cpp 
TEST = bench_main.cpp log.cpp

ALL:
	$(CXX) -g $(CXXFLAGS) $(SRCS) -o  $(TARGET)
	
test:
	$(CXX) -g $(CXXFLAGS) $(TEST) -o  $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	-rm $(TARGET) $(TEST_TARGET) *.o *.log
