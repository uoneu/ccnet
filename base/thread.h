#ifndef CCNET_BASE_THREAD_H
#define CCNET_BASE_THREAD_H

#include "./atomic.h"
#include "./count_down_latch.h"
#include "./types.h"

#include <functional>
#include <memory>
#include <pthread.h>

namespace ccnet
{

class Thread : noncopyable
{
 public:
  typedef std::function<void ()> ThreadFunc; // 函数包装器，用于回调

  explicit Thread(const ThreadFunc&, const string& name = string());
  // FIXME: make it movable in C++11
  explicit Thread(ThreadFunc &&, const string& name = string());
    //右值引用，在对象返回的时候不会拷贝构造临时对象，而是和临时对象交换，提高了效率  
  ~Thread();

  void start(); //启动线程
  int join(); // return pthread_join()

  bool started() const { return started_; } //线程是否启动
  // pthread_t pthreadId() const { return pthreadId_; }
  pid_t tid() const { return tid_; } //线程的真是id
  const string& name() const { return name_; } // 线程的名称

  static int numCreated() { return numCreated_.get(); } //已经启动的线程个数

 private:
  void setDefaultName();

  bool       started_; //线程是否启动 
  bool       joined_; //是否jion
  pthread_t  pthreadId_; //线程的pthread_t
  pid_t      tid_; //线程真实的 pid
  ThreadFunc func_; //线程回调函数
  string     name_; //线程的名称
  CountDownLatch latch_; // 倒计时门闩

  static AtomicInt32 numCreated_; // 原子操作
  // 已经创建的线程的个数，每当创建一个线程，该值就加一(原子整数类)
};


inline void mysleep(int seconds)
{
  timespec t = { seconds, 0 };
  nanosleep(&t, NULL);
}



}
#endif  // CCNET_BASE_THREAD_H
