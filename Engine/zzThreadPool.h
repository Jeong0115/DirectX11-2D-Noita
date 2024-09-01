#pragma once

#include <algorithm>
#include <thread>
#include <mutex>
#include <functional>
#include <atomic>
#include <queue>
#include <future>
#include <deque>
#include <condition_variable>
#include <type_traits>
#include <Windows.h>

namespace zz
{
    class ThreadPool
    {
    private:
        std::vector<std::thread> workers;
        std::queue<std::function<void()>> tasks;
        std::mutex queue_mutex;
        std::condition_variable condition;
        std::condition_variable completed;
        std::atomic<int> active;
        bool stop;

    public:
        ThreadPool(size_t threads) : stop(false), active(0)
        {
            for (size_t i = 0; i < threads; ++i)
            {
                workers.emplace_back(
                    [this/*, i*/] {
                        //DWORD_PTR mask = 1ULL << i;
                        //SetThreadAffinityMask(GetCurrentThread(), mask);
                        for (;;)
                        {
                            std::function<void()> task;
                            {
                                std::unique_lock<std::mutex> lock(this->queue_mutex);
                                this->condition.wait(lock, [this] { return this->stop || !this->tasks.empty(); });
                                if (this->stop && this->tasks.empty()) return;
                                task = std::move(this->tasks.front());
                                ++active;
                                this->tasks.pop();
                            }

                            task();
                            std::unique_lock<std::mutex> lock(queue_mutex);
                            --active;
                            completed.notify_one();
                        }
                    }
                );
            }
        }

        template<class F, class... Args>
        auto enqueue(F&& f, Args&&... args) -> std::future<typename std::invoke_result<F, Args...>::type>
        {
            using return_type = typename std::invoke_result<F, Args...>::type;
            auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

            std::future<return_type> res = task->get_future();
            {
                std::unique_lock<std::mutex> lock(queue_mutex);
                if (stop) throw std::runtime_error("enqueue on stopped ThreadPool");
                tasks.emplace([task]() { (*task)(); });
            }

            condition.notify_one();
            return res;
        }

        ~ThreadPool()
        {
            {
                std::unique_lock<std::mutex> lock(queue_mutex);
                stop = true;
            }
            condition.notify_all();
            for (std::thread& worker : workers) worker.join();
        }

        void wait()
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            completed.wait(lock, [&]() { return tasks.empty() && (active == 0); });
        }

        bool isEmpty()
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            return tasks.empty();
        }
    };
}

