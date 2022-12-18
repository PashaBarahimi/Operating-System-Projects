#ifndef _THREAD_POOL_HPP_
#define _THREAD_POOL_HPP_

#include <functional>
#include <vector>
#include <algorithm>
#include <unordered_map>

#include "thread.hpp"

template <typename T>
class ThreadPool
{
public:
    ThreadPool(int numThreads)
    {
        for (int i = 0; i < numThreads; i++)
            threads_.push_back(new Thread<T>());
    }

    ~ThreadPool()
    {
        for (auto& thread : threads_)
        {
            thread->stop();
            delete thread;
        }
    }

    std::string addTask(int threadId, std::function<void(T&)> task, T& arg)
    {
        std::string id = threads_[threadId]->addTask(task, arg);
        threadTasks_[id] = threadId;
        return id;
    }

    std::string addTask(std::function<void(T&)> task, T& arg)
    {
        int threadId = std::min_element(threads_.begin(), threads_.end(),
            [](Thread<T>* t1, Thread<T>* t2)
            {
                return t1->numTasks() < t2->numTasks();
            }) - threads_.begin();
            return addTask(threadId, task, arg);
    }

    int taskThreadId(const std::string& id)
    {
        return threadTasks_[id];
    }

    bool hasFinished(const std::string& id)
    {
        return threads_[threadTasks_[id]]->hasFinished(id);
    }

    void wait(const std::string& id)
    {
        while (!hasFinished(id))
            usleep(1000);
    }

    void wait(int threadId)
    {
        threads_[threadId]->waitAll();
    }

    void waitAll()
    {
        for (auto& thread : threads_)
            thread->waitAll();
    }

    void stop()
    {
        for (auto& thread : threads_)
            thread->stop();
    }
private:
    std::vector<Thread<T>*> threads_;
    std::unordered_map<std::string, int> threadTasks_;
};

#endif
