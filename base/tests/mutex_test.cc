#include "../count_down_latch.h"
#include "../mutex.h"
#include "../thread.h"
#include "../timestamp.h"

#include <vector>
#include <stdio.h>
#include <iostream>


using namespace ccnet;
using namespace std;

MutexLock g_mutex;
vector<int> g_vec;
const int kCount = 10*1000*1000;

void threadFunc()
{
  for (int i = 0; i < kCount; ++i)
  {
    MutexLockGuard lock(g_mutex);
    g_vec.push_back(i);
  }
}



// foo函数不能作为inline函数优化
int foo() __attribute__ ((noinline));

int g_count = 0;
int foo()
{
  MutexLockGuard lock(g_mutex);
  if (!g_mutex.isLockedByThisThread())
  {
    printf("FAIL\n");
    return -1;
  }

  ++g_count;
  return 0;
}



int main()
{
/////简单锁测试
  MCHECK(foo());
  if (g_count != 1)
  {
    printf("MCHECK calls twice.\n");
    abort();
  }

  const int kMaxThreads = 8;
  g_vec.reserve(kMaxThreads * kCount);


//// 有锁情况插入数据时间测试
  Timestamp start(Timestamp::now());
  for (int i = 0; i < kCount; ++i)
  {
    g_vec.push_back(i);
  }

  printf("single thread without lock %f\n", timeDifference(Timestamp::now(), start));


//// 无锁情况插入数据时间测试
  start = Timestamp::now();
  threadFunc();
  printf("single thread with lock %f\n", timeDifference(Timestamp::now(), start));



//// 多线程情况下使用互斥锁
  for (int nthreads = 1; nthreads < kMaxThreads; ++nthreads)
  {
    std::vector<std::unique_ptr<Thread>> threads;
    g_vec.clear();
    start = Timestamp::now();
    for (int i = 0; i < nthreads; ++i)
    {
      threads.emplace_back(new Thread(threadFunc));
      threads.back()->start();
    }
    for (int i = 0; i < nthreads; ++i)
    {
      threads[i]->join();
    }
    printf("%d thread(s) with lock %f\n", nthreads, timeDifference(Timestamp::now(), start));
  }
}



