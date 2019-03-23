#ifndef CCNET_BASE_CURRENTTHREAD_H
#define CCNET_BASE_CURRENTTHREAD_H

#include <stdint.h>

namespace ccnet
{
// 线程的信息
// POSIX 中的线程是一种轻量级进程，也占用进程号！
// 
// 
namespace current_thread
{
  // internal
  extern __thread int t_cachedTid;  // __thread 线程局部存储
  extern __thread char t_tidString[32]; // 保存tid
  extern __thread int t_tidStringLength; // t_tidString 中tid占用的长度
  extern __thread const char* t_threadName; // 线程名称
  void cacheTid();

  inline int tid() // 线程tid
  {
    if (__builtin_expect(t_cachedTid == 0, 0))
    {
      cacheTid();
    }
    return t_cachedTid;
  }

  inline const char* tidString() // for logging
  {
    return t_tidString;
  }

  inline int tidStringLength() // for logging
  {
    return t_tidStringLength;
  }

  inline const char* name()
  {
    return t_threadName;
  }

  bool isMainThread(); // 主线程的id和进程的id一致

  void sleepUsec(int64_t usec); // 微秒
}

}

#endif
