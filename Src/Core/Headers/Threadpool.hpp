#pragma once
//Copied from geeksforgeeks

#include <condition_variable> 
#include <functional> 
#include <mutex> 
#include <queue> 
#include <thread> 
  
// Class that represents a simple thread pool 
template<typename TaskType>
class ThreadPool { 
public: 
    // // Constructor to creates a thread pool with given 
    // number of threads 
    ThreadPool(size_t num_threads 
               = std::thread::hardware_concurrency()) 
    {
        // Creating worker threads 
        for (size_t i = 0; i < num_threads; ++i) { 
            threads_.emplace_back([this] { 
                while (true) { 
                    std::function<TaskType> task; 
                    // The reason for putting the below code 
                    // here is to unlock the queue before 
                    // executing the task so that other 
                    // threads can perform enqueue tasks 
                    { 
                        // Locking the queue so that data 
                        // can be shared safely 
                        std::unique_lock<std::mutex> lock( 
                            queue_mutex_); 
  
                        // Waiting until there is a task to 
                        // execute or the pool is stopped 
                        cv_.wait(lock, [this] { 
                            return !tasks_.empty() || stop_; 
                        }); 
  
                        // exit the thread in case the pool 
                        // is stopped and there are no tasks 
                        if (stop_ && tasks_.empty()) { 
                            return; 
                        } 
  
                        // Get the next task from the queue 
                        task = std::move(tasks_.front()); 
                        tasks_.pop(); 
                    } 
  
                    task(); 
                } 
            }); 
        } 
    }

    template<typename ArgType>
    ThreadPool(ArgType* args, size_t num_threads 
               = std::thread::hardware_concurrency()) 
    {
        // Creating worker threads 
        for (size_t i = 0; i < num_threads; ++i) { 
            threads_.emplace_back([this, arg = args[i]] { 
                while (true) { 
                    std::function<TaskType> task; 
                    // The reason for putting the below code 
                    // here is to unlock the queue before 
                    // executing the task so that other 
                    // threads can perform enqueue tasks 
                    { 
                        // Locking the queue so that data 
                        // can be shared safely 
                        std::unique_lock<std::mutex> lock( 
                            queue_mutex_); 
  
                        // Waiting until there is a task to 
                        // execute or the pool is stopped 
                        cv_.wait(lock, [this] { 
                            return !tasks_.empty() || stop_; 
                        }); 
  
                        // exit the thread in case the pool 
                        // is stopped and there are no tasks 
                        if (stop_ && tasks_.empty()) { 
                            return; 
                        } 
  
                        // Get the next task from the queue 
                        task = std::move(tasks_.front()); 
                        tasks_.pop(); 
                    } 
  
                    task(arg); 
                } 
            }); 
        } 
    }

    // Destructor to stop the thread pool 
    ~ThreadPool() 
    { 
        { 
            // Lock the queue to update the stop flag safely 
            std::unique_lock<std::mutex> lock(queue_mutex_); 
            stop_ = true; 
        } 
  
        // Notify all threads 
        cv_.notify_all(); 
  
        // Joining all worker threads to ensure they have 
        // completed their tasks 
        for (auto& thread : threads_) { 
            thread.join(); 
        } 
    } 
  
    // Enqueue task for execution by the thread pool 
    virtual void enqueue(std::function<TaskType> task) 
    { 
        { 
            std::unique_lock<std::mutex> lock(queue_mutex_); 
            tasks_.emplace(std::move(task)); 
        } 
        cv_.notify_one(); 
    }
  
protected: 
    // Vector to store worker threads 
    std::vector<std::thread> threads_; 
  
    // Queue of tasks 
    std::queue<std::function<TaskType>> tasks_; 
  
    // Mutex to synchronize access to shared data 
    std::mutex queue_mutex_; 
  
    // Condition variable to signal changes in the state of 
    // the tasks queue 
    std::condition_variable cv_; 
  
    // Flag to indicate whether the thread pool should stop 
    // or not 
    bool stop_ = false; 
}; 