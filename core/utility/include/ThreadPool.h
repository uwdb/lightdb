#ifndef LIGHTDB_THREADPOOL_H
#define LIGHTDB_THREADPOOL_H

#include "GPUContext.h"
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <functional>


class ThreadPool {
public:
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool(const ThreadPool&&) = delete;

    ThreadPool(const size_t threads)
        : ThreadPool(threads, std::bind(&ThreadPool::Worker, this))
    { }

    ~ThreadPool() {
        terminate_ = true;
        condition_.notify_all();
        for (auto& worker: workers_) {
            worker.join();
        }
    }

    void push(const std::function<void()> &task) {
        {
            std::unique_lock{mutex_};
            taskQueue_.push(task);
            taskCount_++;
        }
        condition_.notify_one();
    }

    void waitAll() const {
        while (taskCount_ != 0u) {
            std::this_thread::yield();
        }
    }

protected:
    ThreadPool(const size_t threads, std::function<void()> worker)
        : taskCount_(0), terminate_(false)
    {
        std::generate_n(std::back_inserter(workers_), threads, [worker] {
            return std::thread(worker); });
    }

    void Worker() {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock lock{mutex_};
                condition_.wait(lock, [this]() {
                    return !taskQueue_.empty() || terminate_;
                });

                if (terminate_ && taskQueue_.empty()) {
                    return;
                }

                task = std::move(taskQueue_.front());
                taskQueue_.pop();
            }

            task();
            taskCount_--;
        }
    }

private:
    std::vector<std::thread>            workers_;
    std::queue<std::function<void()>>   taskQueue_;
    std::atomic_uint                    taskCount_;
    std::mutex                          mutex_;
    std::condition_variable             condition_;
    std::atomic_bool                    terminate_;
};


class GPUThreadPool : public ThreadPool {
public:
    GPUThreadPool(GPUContext &context, const size_t threads)
        : ThreadPool(threads, std::bind(&GPUThreadPool::Worker, this, &context))
    { }

protected:
    void Worker(const GPUContext *context) {
        context->AttachToThread();
        ThreadPool::Worker();
    }
};

#endif //LIGHTDB_THREADPOOL_H
