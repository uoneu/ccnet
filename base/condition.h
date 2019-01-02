#ifndef CCNET_BASE_CONDITION_H
#define CCNET_BASE_CONDITION_H

#include <pthread.h>

#include "./mutex.h"

namespace ccnet
{

/// 条件变量
/// 条件变量給多个线程提供了一个会和的场所
///

class Condition : noncopyable
{
 public:
  explicit Condition(MutexLock& mutex)
    : mutex_(mutex)
  {
    MCHECK(pthread_cond_init(&pcond_, NULL));
  }

  ~Condition()
  {
    MCHECK(pthread_cond_destroy(&pcond_));
  }

  void wait() // pthread_cond_wait
  {
    MutexLock::UnassignGuard ug(mutex_); //pthread_cond_wait内部会改变锁的状态, unlock,
    MCHECK(pthread_cond_wait(&pcond_, mutex_.getPthreadMutex()));
  }

  // returns true if time out, false otherwise.
  bool waitForSeconds(double seconds); // pthread_cond_timewait

  void notify() // 单播， 不会有“惊群现象”产生，他最多只给一个线程发信号
  {
    MCHECK(pthread_cond_signal(&pcond_));
  }

  void notifyAll() // 广播
  {
    MCHECK(pthread_cond_broadcast(&pcond_));
  }

 private:
  MutexLock& mutex_;     // 锁
  pthread_cond_t pcond_;  // 条件
};

}




#endif  // CCNET_BASE_CONDITION_H
