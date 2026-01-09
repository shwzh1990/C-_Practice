#include <atomic>
#include <cstddef>
#include <iostream>
#include <utility>
#include <vector>
#include <assert.h>
template<class T>
class ringbuff
{
	public:
  	explicit ringbuff(size_t capacity = 512);

		~ringbuff() = default;
	
    template<typename U>
		bool put(U &&s);
		bool pop(T &s);
	private:
		alignas(64) std::atomic<size_t> write_pos_;
		alignas(64) std::atomic<size_t> read_pos_;
		size_t capacity_;
		std::vector<T> buff_;
    std::vector<std::atomic<bool>> ready_flags_;

		size_t next_pos(size_t current_pos);


};
template<typename T>
ringbuff<T>::ringbuff(size_t capacity) : write_pos_{0}, read_pos_{0}, capacity_(capacity), buff_(capacity_ + 1), ready_flags_(capacity_ + 1)
{
  assert(capacity > 1);
}

template<typename T>
template<typename U>
bool ringbuff<T>:: put(U &&s)
{
  size_t current_write_pos = 0;
  size_t next_write_pos = 0;
  
  do
  {
   current_write_pos = write_pos_.load(std::memory_order_relaxed);
   next_write_pos = next_pos(current_write_pos);
  if(next_write_pos == read_pos_.load(std::memory_order_acquire))   //waiting for read_pos_ read the lastest read pos.
     {
       return false;
     }
  }while(!write_pos_.compare_exchange_weak(current_write_pos, next_write_pos, std::memory_order_acq_rel, std::memory_order_relaxed));
  buff_[current_write_pos] = std::string(std::forward<U>(s));  //here we need get the next_write_pos at first to make sure the thread is here and then the data can be write in safely.
  
  ready_flags_[current_write_pos].store(true, std::memory_order_release);
  return true;

}

template<typename T>
bool ringbuff<T>::pop(T& s)
{
  size_t r = read_pos_.load(std::memory_order_relaxed);
  
  if(!ready_flags_[r].load(std::memory_order_acquire))  //is empty.
  {
    return false;
  }
  s = std::move(buff_[r]);
  ready_flags_[r].store(false, std::memory_order_relaxed);
  size_t next_read_pos = next_pos(r);
  read_pos_.store(next_read_pos, std::memory_order_release);
  return true;


}


template<typename T>
size_t ringbuff<T>::next_pos(size_t current_pos)
{
  return (current_pos + 1) % (capacity_ + 1);
}
