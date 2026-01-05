## 你现在的位置（已完成 ✅）

### ✅ 阶段 1：高性能 RingBuffer

你已经练到了这些能力（很重要）：

- `atomic` + `memory_order`（relaxed / acquire / release）
- SPSC / 接近 lock-free 的设计思维
- cache line / false sharing 的意识
- 用 perf / benchmark 看性能差异（你已经有感觉了）

**结论：你已经不是“写 STL 用法”的 C++ 水平了，而是“系统级 C++”的门口**

------

# 后续 3 个核心项目（这是我们当初约定的）

下面是 **RingBuffer 之后必做的 3 个项目**，每一个都明显“上一个台阶”。

------

## 🚀 项目 2（第 2 周）：异步日志系统（Async Logger）

### 为什么下一步一定是它？

因为：

> **RingBuffer 是工具，Async Logger 是完整系统**

你会第一次把：

- 多线程
- 内存模型
- IO
- 性能测试
   **全部绑在一个真实工程里**

------

### 项目目标

实现一个工业级思路的日志系统：

```

业务线程
   |
   v
RingBuffer（无锁 / 低锁）
   |
   v
日志线程
   |
   v
磁盘（文件）
```

------

### 必须包含的技术点（硬性要求）

- **多生产者 / 单消费者（MPSC）**
- 日志线程独占 IO
- 日志线程 `fwrite / write`
- 支持：
  - 日志等级
  - 时间戳
  - 批量 flush
- **禁止 mutex 包裹整个 push**

------

### 你在这个项目会真正搞懂：

- 为什么 release / acquire 是“线程间通信”
- 为什么 `std::cout` 在多线程里是灾难
- 为什么工业日志系统几乎都是 async

------

### Benchmark 要求

你必须对比：

1. 直接 `std::cout`
2. mutex + vector
3. 你的 async logger

👉 用 `perf` 看：

- syscalls
- context switch
- cache miss

------

## 🔥 项目 3（第 3 周）：线程池 + Task System（不是玩具）

### 注意：这一步是“分水岭”

**99% C++ 学习者到不了这里**

------

### 项目目标

你要写的是 **Server 级线程池**，而不是教程那种。

```

submit(task)
   |
   v
work stealing / queue
   |
   v
worker threads
```

------

### 强制技术点

- `std::function` vs 模板 task（你要比较）
- `future / promise` **或自己实现简化版**
- **每线程本地队列 + 全局队列**
- 尽量减少 contention

------

### 你会学到：

- 为什么线程池不是“多开几个线程”
- 为什么很多系统宁愿 task queue + 单线程 IO
- cache locality 在并发里的决定性作用

------

### Benchmark

- 单任务 vs 批量任务
- 不同线程数
- mutex 队列 vs lock-free 队列

------

## 🧠 项目 4（第 4 周）：高性能 Server Skeleton（真正工业向）

这是**压轴项目**。

------

### 项目目标

写一个 **Linux 下的高性能服务器骨架**：

- `epoll`
- Reactor 模型
- IO 线程 + worker 线程池
- 无锁 / 少锁数据通道

```

epoll thread
   |
   v
task queue
   |
   v
worker threads
```

------

### 必须涉及

- `epoll`
- 非阻塞 socket
- IO 与计算分离
- 你前面写的：
  - RingBuffer
  - Logger
  - ThreadPool

👉 **这是把你前 3 周成果“整合”的项目**