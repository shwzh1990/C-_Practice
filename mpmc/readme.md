# __MPSC 无锁的核心思路__

首先先说一些概念：
- [__MPSC 无锁的核心思路__](#mpsc-无锁的核心思路)
	- [什么是无锁](#什么是无锁)
		- [原子操作的函数解析](#原子操作的函数解析)
	- [什么是CAS](#什么是cas)
	- [MPSC 是怎么做的](#mpsc-是怎么做的)

## 什么是无锁
所谓无锁指的是不使用 mutex 或者是conditional var. 因为他们的底层都是需要调用context switch。任务的切换是需要消耗大量的时间，在高并发的环境中这种切换成本是昂贵的。所以无锁的思维就产生了。所谓的无锁指的是利用cpu的原子操作来实现写和读。在ringbuf中 一个线程或者多个线程写ringbuff 一个线程读出ringbuf的数据 写入文件。但是其实写和读的部分并不是同一个部分(一个数组的不同的部分)。当这种情况下其实读和写不是同一个地方的时候我们就可以使用原子操作来解决这个问题。
### 原子操作的函数解析 
这里用这么几个原子操作的函数：
   - memory_order_relaxed : 这个参数只是保证了这个原子变量的自己的原子性 所谓的原子性就是在一个时钟周期做加减
   - memory_order_require: 这个参数保证了后面的命令执行不会比这个命令执行提前。
   - memory_order_release: 这个参数保证了前面的命令必须在执行原子操作之前完成
  
这里有一个概念叫做cpu乱序：
因为cpu为了优化设计所以在执行代码的时候顺序会故意打乱(具体原理未知) 这就有可能让原子操作执行的顺序和前面后面的程序会乱。所以我们需要上面的三个参数强行的设定好原子操作和前后程序的顺序 也就是说比如write pos必须在数据写入了buffer之后才能加一 或者是read pos在数据已经拿出去之后才能加一。
## 什么是CAS
CAS = Compare And Swap
中文可以叫做：比较并交换。

它的操作流程是：

读 一个内存位置 addr 的当前值 old_value

比较 当前值是否等于你期望的值 expected

如果相等 → 把内存改成新值 new_value

如果不相等 → 不修改，告诉你操作失败
```
bool compare_exchange_weak(T& expected, T desired,
                           std::memory_order success = std::memory_order_seq_cst,
                           std::memory_order failure = std::memory_order_seq_cst);
```
这个是cpu的原子操作 我们的weak函数允许失败 然后自旋一段时间。

这个是多生产者 单消费者的基石。
## MPSC 是怎么做的


```
template<typename T>
template<typename U>
bool ringbuff<T>:: put(U &&s)
{
  size_t current_write_pos = write_pos_.load(std::memory_order_relaxed);
  size_t next_write_pos = next_pos(current_write_pos);
  do
  {
  if(next_write_pos == read_pos_.load(std::memory_order_acquire))   //waiting for read_pos_ read the lastest read pos.
     {
       return false;
     }
  }while(!write_pos_.compare_exchange_weak(current_write_pos, next_write_pos, std::memory_order_acq_rel));
  buff_[current_write_pos] = std::forward<U>(s);  //here we need get the next_write_pos at first to make sure the thread is here and then the data can be write in safely.
  return true;

}
```
其中注意的是 compare_exchange_weak 如果失败了那么也会把write_pos 的值变成expect的值 因为其他的线程更新了这个值。


__请注意__:
在多线程中 由于CPU每次都会读取64字节来， 所以如果不同的线程写同一个内存 如果几个线程写的内容小于64字节就会造成share falsing
所以说我们需要把一些写的内存的地址偏移offset是64字节 从而缓存不会失效 。