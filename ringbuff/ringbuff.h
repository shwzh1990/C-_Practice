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
	
		bool put(T &&s);
		bool pop(T &s);
	private:
		alignas(64) std::atomic<size_t> write_pos_;
		alignas(64) std::atomic<size_t> read_pos_;
		size_t capacity_;
		std::vector<T> buff_;

		size_t next_pos(size_t current_pos);


};
template<typename T>
ringbuff<T>::ringbuff(size_t capacity) : write_pos_{0}, read_pos_{0}, capacity_(capacity), buff_(capacity_)
{
  assert(capacity > 1);
}

template<typename T>
bool ringbuff<T>:: put(T &&s)
{
  size_t w = write_pos_.load(std::memory_order_relaxed);
  size_t next_write_pos = next_pos(w);
  if(next_write_pos == read_pos_.load(std::memory_order_acquire))   //waiting for read_pos_ read the lastest read pos.
     {
       return false;
     }
   buff_[w] = std::forward<T>(s);
   write_pos_.store(next_write_pos, std::memory_order_release);
   return true;

}

template<typename T>
bool ringbuff<T>::pop(T& s)
{
  size_t r = read_pos_.load(std::memory_order_relaxed);
  
  if(r == write_pos_.load(std::memory_order_acquire))  //is empty.
  {
    return false;
  }
  s = std::move(buff_[r]);
  size_t next_read_pos = next_pos(r);
  read_pos_.store(next_read_pos, std::memory_order_release);
  return true;


}


template<typename T>
size_t ringbuff<T>::next_pos(size_t current_pos)
{
  return (current_pos + 1) % capacity_;
}
