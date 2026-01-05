High-Performance Lock-Free SPSC Ring Buffer (ringbuff)
Overview
This is a high-performance, Lock-Free Single-Producer Single-Consumer (SPSC) ring buffer implemented in C++11/14/17. It is specifically engineered for scenarios demanding ultra-low latency and extreme throughput, such as real-time data processing, High-Frequency Trading (HFT) systems, network packet capture, or asynchronous logging.

Key Features:

Lock-Free Design: Eliminates the overhead of mutexes, condition variables, and system calls.

Memory Ordering Optimization: Utilizes fine-grained atomic operations with std::memory_order_acquire, std::memory_order_release, and std::memory_order_relaxed.

Cache Line Alignment: Prevents False Sharing using alignas(64) to ensure independent cache lines for hot atomic variables.

Extreme Throughput: Capable of achieving over 89+ Million Operations per Second (Mops/s) on mobile/laptop hardware.

Why Lock-Free?
In multi-threaded programming, traditional locking mechanisms (e.g., std::mutex) often become the primary bottleneck in high-concurrency scenarios. Locks involve thread suspension, context switching, and contention overhead. Lock-free programming leverages atomic operations and memory barriers, allowing threads to access shared data concurrently without blocking each other, resulting in significantly higher throughput and lower tail latency.

Technical Highlights
1. Eliminating False Sharing
A critical optimization for lock-free queues. When read_pos_ and write_pos_ (frequently modified by different CPU cores) reside on the same CPU Cache Line, it triggers constant cache invalidations and synchronization across cores, drastically degrading performance.

We use alignas(64) to force these variables onto distinct cache lines:

C++

template<typename T>
class ringbuff {
    // ...
    alignas(64) std::atomic<size_t> write_pos_;
    alignas(64) std::atomic<size_t> read_pos_;
    // ...
};
2. Precise Memory Barrier Control
To ensure data visibility and strict ordering between the producer and consumer, we employ a tailored memory model:

std::memory_order_release: The producer updates write_pos_ with release semantics after writing data, ensuring all prior writes are visible to other threads.

std::memory_order_acquire: The consumer loads read_pos_ with acquire semantics before reading, ensuring it sees all data written before the producer's release.

std::memory_order_relaxed: Used for internal operations (e.g., loading a thread's own position) that do not impact cross-thread synchronization.

3. Modulo Operation Optimization
When the buffer capacity is a power of 2, expensive modulo operations are replaced with bitwise AND operations to accelerate index calculation:

C++

// Assuming capacity_ = 1024
// return (current_pos + 1) % capacity_; // Standard
return (current_pos + 1) & (capacity_ - 1); // Optimized
4. Spin-Wait Optimization (__builtin_ia32_pause())
Within the spin-wait loops of put and pop, we incorporate the PAUSE instruction. This hints to the CPU that the thread is in a busy-wait state, which:

Reduces CPU power consumption and heat generation.

Prevents pipeline flushes caused by memory order violations during speculatively executed loops.

Improves performance for the sibling hardware thread in hyper-threaded environments.

C++

while (!rb.put(std::move(i))) {
    // Hint to CPU for power-efficient and high-performance spin-waiting
    __builtin_ia32_pause();
}
Performance Benchmarks
Benchmark performed on size_t types with 10 million put/pop operations.

Environment:

CPU: 11th Gen Intel(R) Core(TM) i7-1165G7 @ 2.80GHz

Memory: 32.0 GB (31.7 GB usable)

Compiler: GCC 11.4.0 (Optimized with -O3 -pthread)

OS: Linux (Ubuntu 22.04 LTS / WSL2)

Results:

Plaintext

Finish 10,000,000th operation
Total time elapsed: 0.112224s
Throughput: 89.1074 Million ops/s
