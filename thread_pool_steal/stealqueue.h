#ifndef __STEALQUEUE_H
#define __STEALQUEUE_H

#include <deque>
#include <functional>
#include <memory>
#include <optional>
#include <mutex>
#include <condition_variable>
#include <thread>

using Task = std::function<void()>;
class WorkStealingPool {
    // 每个线程私有的任务队列（带锁保护）
    struct LocalQueue {
        std::deque<Task> tasks;
        std::mutex mtx;

        void push(Task t) {
            std::lock_guard<std::mutex> lock(mtx);
            tasks.push_front(std::move(t)); // 靠近顶部，方便自己取
        }

        bool try_pop(Task& t) {
            std::lock_guard<std::mutex> lock(mtx);
            if (tasks.empty()) return false;
            t = std::move(tasks.front());
            tasks.pop_front();
            return true;
        }

        bool try_steal(Task& t) {
            std::lock_guard<std::mutex> lock(mtx);
            if (tasks.empty()) return false;
            t = std::move(tasks.back()); // 窃取末尾的任务
            tasks.pop_back();
            return true;
        }
    };

    std::vector<std::unique_ptr<LocalQueue>> queues;
    std::vector<std::thread> workers;
    std::atomic<bool> stop{false};
    int thread_count;

public:
    WorkStealingPool(int n) : thread_count(n) {
        for (int i = 0; i < n; ++i) {
            queues.push_back(std::make_unique<LocalQueue>());
        }

        for (int i = 0; i < n; ++i) {
            workers.emplace_back([this, i] {
                while (!stop) {
                    Task t;
                    bool found = false;

                    // 1. 尝试从自己的队列拿任务
                    if (queues[i]->try_pop(t)) {
                        found = true;
                    } 
                    // 2. 自己的拿完了，尝试偷别人的
                    else {
                        for (int j = 0; j < thread_count; ++j) {
                            if (i == j) continue; // 不偷自己
                            if (queues[j]->try_steal(t)) {
                                LOG_INFO("the thread %d steal thread %d", i, j); 
                                found = true;
                                break;
                            }
                        }
                    }

                    if (found) {
                        t(); // 执行任务
                    } else {
                        std::this_thread::yield(); // 没活干，让出 CPU
                    }
                }
            });
        }
    }

    // 简单地把任务塞给某个线程（实际应有更复杂的负载均衡）
    void submit(int thread_id, Task t) {
        queues[thread_id % thread_count]->push(std::move(t));
    }

    ~WorkStealingPool() {
        stop = true;
        for (auto& w : workers) w.join();
    }
};
#endif
