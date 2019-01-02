#ifndef CCNET_BASE_THREADPOOL_H
#define CCNET_BASE_THREADPOOL_H

#include"./condition.h"
#include "./mutex.h"
#include "./thread.h"
#include "./types.h"

#include <deque>
#include <vector>

namespace ccnet
{

// 线程池管理
// 包含线程队列和任务队列，本质是消费者和生产者模式，线程取任务去执行
//

class ThreadPool : noncopyable
{
 public:
  typedef std::function<void ()> Task;

  explicit ThreadPool(const string& nameArg = string("ThreadPool"));
  ~ThreadPool();

  // 下面的2函数必须在start()函数之前运行
  void setMaxQueueSize(int maxSize) { maxQueueSize_ = maxSize; } ////设置任务队列的大小，0表示无界队列（默认）
  void setThreadInitCallback(const Task& cb) // 设置线程的初始化函数
  { threadInitCallback_ = cb; }


  void start(int numThreads); // 启动线程池（启动时配置线层池大小）
  void stop();  // 停止线程池，要等待所有线程执行完

  const string& name() const // 线程池名称
  { return name_; }

  size_t queueSize() const; // 任务队列的大小

  // Could block if maxQueueSize > 0
  void run(Task f); // 将任务f加入任务队列(等待的线程会取任务执行）

 private:
  bool isFull() const; // 任务队列满？
  void runInThread(); //线程运行函数，从队列中取任务执行
  Task take(); // 获取任务队列中的任务
 
  mutable MutexLock mutex_; // mutable的，表示在const函数中也可以改变它
  Condition notEmpty_; // 不空条件（消费者）
  Condition notFull_; //  不满条件（生产者），用于生产者和消费者之间的同步
  string name_; // 线程池名字
  Task threadInitCallback_; // 线程初始化回调函数
  std::vector<std::unique_ptr<ccnet::Thread>> threads_; // 线程集合
  std::deque<Task> queue_; // 任务队列
  size_t maxQueueSize_; // 任务队列的大小
  bool running_; // 标志是否start启动了，或者stop 停止了
};

}

#endif // CCNET_BASE_THREADPOOL_H
