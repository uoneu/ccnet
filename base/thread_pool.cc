#include "./thread_pool.h"

#include "./exception.h"
#include "./logging.h"

#include <assert.h>
#include <stdio.h>

using namespace ccnet;


ThreadPool::ThreadPool(const string& nameArg)
  : mutex_(),
    notEmpty_(mutex_),
    notFull_(mutex_),
    name_(nameArg),
    maxQueueSize_(0),
    running_(false)
{
}


ThreadPool::~ThreadPool()
{
  if (running_)
  {
    stop();
  }
}


// 启动线程池（启动时配置线层池大小）
void ThreadPool::start(int numThreads)
{
  assert(threads_.empty());
  running_ = true;
  threads_.reserve(numThreads);
  for (int i = 0; i < numThreads; ++i)
  {
    char id[32];
    snprintf(id, sizeof id, "%d", i+1);
    threads_.emplace_back(new ccnet::Thread(
          std::bind(&ThreadPool::runInThread, this), name_+id));
    threads_[i]->start();
  }
  if (numThreads == 0 && threadInitCallback_) 
  {
    threadInitCallback_();
  }
}


// 停止线程池，要等待所有线程执行完
void ThreadPool::stop()
{
  {
  MutexLockGuard lock(mutex_);
  running_ = false;
  notEmpty_.notifyAll(); // 唤醒所有等待的线程
  }
  for (auto& thr : threads_)
  {
    thr->join();
  }
}


// 任务队列的大小
size_t ThreadPool::queueSize() const
{
  MutexLockGuard lock(mutex_);
  return queue_.size();
}


// 将任务f加入线程池, 并运行
void ThreadPool::run(Task task)
{
  if (threads_.empty()) // 只有一个主线程
  {
    task();
  }
  else
  {
    MutexLockGuard lock(mutex_);
    while (isFull()) // 如果task队列queue_满了，就等待
    {
      notFull_.wait();
    }
    assert(!isFull());

    queue_.push_back(std::move(task));
    notEmpty_.notify();
  }
}


// 获取任务队列 queue_ 中的任务 即run中添加的f 
ThreadPool::Task ThreadPool::take()
{
  MutexLockGuard lock(mutex_);
  // always use a while-loop, due to spurious wakeup
  LOG_INFO << queue_.size();
  while (queue_.empty() && running_)
  {
    notEmpty_.wait();
    LOG_INFO << queue_.size();
	
  }
  Task task;
  if (!queue_.empty())
  {
    task = queue_.front();
    queue_.pop_front();
    if (maxQueueSize_ > 0) // 为0表示无限队列
    {
      notFull_.notify();
    }
  }
  return task;
}


// 任务队列满？
bool ThreadPool::isFull() const
{
  mutex_.assertLocked();
  return maxQueueSize_ > 0 && queue_.size() >= maxQueueSize_;
}


// 线程创建时所传入的运行函数: --> take()--> f() 即运行run中添加的任务f  
void ThreadPool::runInThread()
{
  try
  {
    if (threadInitCallback_) // 如果设置了，就执行它，进行一些初始化设置
    {
      threadInitCallback_();
    }

    while (running_) // 循环取任务执行
    {
      Task task(take()); // 从任务队列取出一个任务，执行它
      if (task)
      {
        task();
      }
    }
  }
  catch (const Exception& ex)
  {
    fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
    fprintf(stderr, "reason: %s\n", ex.what());
    fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
    abort();
  }
  catch (const std::exception& ex)
  {
    fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
    fprintf(stderr, "reason: %s\n", ex.what());
    abort();
  }
  catch (...)
  {
    fprintf(stderr, "unknown exception caught in ThreadPool %s\n", name_.c_str());
    throw; // rethrow
  }
}

