#ifndef _THREAD_HPP_
#define _THREAD_HPP_

#include <functional>
#include <queue>
#include <unordered_set>
#include <random>
#include <unistd.h>
#include <pthread.h>

constexpr int ID_LENGTH = 8;

template <typename T>
class Thread
{
public:
    Thread(): running_(true), gen_(std::random_device()()), dis_(0, 1000000)
    {
        pthread_mutex_init(&finishedMutex_, NULL);
        pthread_mutex_init(&tasksMutex_, NULL);
        pthread_cond_init(&tasksCond_, NULL);
        pthread_create(&thread_, NULL, run, this);
    }

    ~Thread()
    {
        running_ = false;
        pthread_cond_signal(&tasksCond_);
        pthread_join(thread_, NULL);
        pthread_mutex_destroy(&finishedMutex_);
        pthread_mutex_destroy(&tasksMutex_);
        pthread_cond_destroy(&tasksCond_);
    }

    std::string addTask(std::function<void(T&)> task, T& arg)
    {
        std::string id = generateId();
        pthread_mutex_lock(&tasksMutex_);
        tasks_.push({ id, task, arg });
        pthread_cond_signal(&tasksCond_);
        pthread_mutex_unlock(&tasksMutex_);
        return id;
    }

    int numTasks()
    {
        pthread_mutex_lock(&tasksMutex_);
        int numTasks = tasks_.size();
        pthread_mutex_unlock(&tasksMutex_);
        return numTasks;
    }

    bool hasFinished(const std::string& id)
    {
        pthread_mutex_lock(&finishedMutex_);
        bool finished = finished_.find(id) != finished_.end();
        pthread_mutex_unlock(&finishedMutex_);
        return finished;
    }

    void wait(const std::string& id)
    {
        while (!hasFinished(id))
            usleep(1000);
    }

    void waitAll()
    {
        while (true)
        {
            pthread_mutex_lock(&tasksMutex_);
            bool empty = tasks_.empty();
            pthread_mutex_unlock(&tasksMutex_);
            if (empty)
                break;
            usleep(1000);
        }
    }

    void stop()
    {
        running_ = false;
        pthread_cond_signal(&tasksCond_);
        pthread_join(thread_, NULL);
    }

private:
    struct Task
    {
        std::string id;
        std::function<void(T&)> task;
        T& arg;
    };

    std::queue <Task> tasks_;
    std::unordered_set<std::string> finished_;
    pthread_mutex_t finishedMutex_;
    pthread_mutex_t tasksMutex_;
    pthread_cond_t tasksCond_;
    pthread_t thread_;
    bool running_;
    std::mt19937 gen_;
    std::uniform_int_distribution<> dis_;

    static void* run(void* arg)
    {
        Thread* thread = (Thread*)arg;
        while (thread->running_)
        {
            pthread_mutex_lock(&thread->tasksMutex_);
            while (thread->tasks_.empty() && thread->running_)
                pthread_cond_wait(&thread->tasksCond_, &thread->tasksMutex_);
            if (!thread->running_)
            {
                pthread_mutex_unlock(&thread->tasksMutex_);
                break;
            }
            Task task = thread->tasks_.front();
            thread->tasks_.pop();
            pthread_mutex_unlock(&thread->tasksMutex_);
            task.task(task.arg);
            pthread_mutex_lock(&thread->finishedMutex_);
            thread->finished_.insert(task.id);
            pthread_mutex_unlock(&thread->finishedMutex_);
        }
        return NULL;
    }

    std::string generateId()
    {
        static const char alphanum[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
        std::string id;
        for (int i = 0; i < ID_LENGTH; ++i)
            id += alphanum[dis_(gen_) % (sizeof(alphanum) - 1)];
        return id;
    }
};

#endif
