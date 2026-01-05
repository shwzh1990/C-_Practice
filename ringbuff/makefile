CXX = g++
CXXFLAGS = -std=c++14 -O3 -Wall -Wextra 

# 目标可执行文件
TARGET = ringbuf

TEST_TARGET = ringbuff_bench_test
# 源文件
SRCS = main.cpp 
TEST = bench_main.cpp

ALL:
	$(CXX) -g $(CXXFLAGS) $(SRCS) -o  $(TARGET)
	
test:
	$(CXX) -g $(CXXFLAGS) $(TEST) -o  $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm $(TARGET) $(TEST_TARGET) *.o
