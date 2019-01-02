#ifndef CCNET_BASE_COUNTDOWNLATCH_H
#define CCNET_BASE_COUNTDOWNLATCH_H

#include "./condition.h"
#include "./mutex.h"

namespace ccnet
{
/// “倒计时门闩”同步, 所有线程等待倒计时结束
/// 使用条件变量
/// 
class CountDownLatch : noncopyable
{
 public:

  explicit CountDownLatch(int count); // 初始时间

  void wait(); // 等待计时为0， 若为0，通知所有等待的进程

  void countDown(); // 倒计时 -1

  int getCount() const; // 时间

 private:
  mutable MutexLock mutex_;
  Condition condition_;
  int count_;
};

}
#endif  // CCNET_BASE_COUNTDOWNLATCH_H
