// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "./current_thread.h"


#include <type_traits>

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <linux/unistd.h>
#include <time.h>


namespace ccnet 
{
//
//
//
namespace current_thread 
{
  __thread int t_cachedTid = 0;
  __thread char t_tidString[32];
  __thread int t_tidStringLength = 6;
  __thread const char* t_threadName = "unknown";
  static_assert(std::is_same<int, pid_t>::value, "pid_t should be int");
}

//
//
namespace detail
{
  pid_t gettid()
  { 
    // 真实的线程id唯一标识, 若使用pthread_self()， 可能存在重复
    return static_cast<pid_t>(::syscall(SYS_gettid)); 
  }

}

} // namespace ccnet




using namespace ccnet;


void current_thread::cacheTid()
{
  if (t_cachedTid == 0)
  {
    t_cachedTid = detail::gettid();
    t_tidStringLength = snprintf(t_tidString, sizeof t_tidString, "%5d ", t_cachedTid);
  }
}


bool current_thread::isMainThread()
{
  return tid() == ::getpid(); // 
}


void current_thread::sleepUsec(int64_t usec)
{
  struct timespec ts = { 0, 0 };
  const int kMicroSecondsPerSecond = 1000 * 1000;
  ts.tv_sec = static_cast<time_t>(usec / kMicroSecondsPerSecond);
  ts.tv_nsec = static_cast<long>(usec % kMicroSecondsPerSecond * 1000);
  ::nanosleep(&ts, nullptr);

}

