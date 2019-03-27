#include "../thread_pool.h"
#include "../count_down_latch.h"
#include "../current_thread.h"
#include "../thread.h"
#include "../logging.h"
#include "../time_zone.h"

#include <stdio.h>
#include <unistd.h>  // usleep
#include <iostream>

void print()
{
  printf("tid=%d\n", ccnet::current_thread::tid());
}

void printString(const std::string& str)
{
  //LOG_INFO << str;
  //printf("%s\n",str.data());
  usleep(100*1000);
}

void test(int maxSize)
{
 // LOG_WARN << "Test ThreadPool with max queue size = " << maxSize;
  ccnet::ThreadPool pool("MainThreadPool");
  pool.setMaxQueueSize(maxSize);
  pool.start(5);

  // LOG_WARN << "Adding";
  pool.run(print);
  pool.run(print);
  pool.run(print);
  pool.run(print);
  for (int i = 0; i < 100; ++i)
  {
    char buf[32];
    snprintf(buf, sizeof buf, "task %d", i);
    pool.run(std::bind(printString, std::string(buf)));
  }
  // LOG_WARN << "Done";

  ccnet::CountDownLatch latch(1);
  pool.run(std::bind(&ccnet::CountDownLatch::countDown, &latch)); // 参数绑定
  latch.wait();
  pool.stop();
}


using namespace std;

void task1() {
  cout << "task1...runing\n";
  ccnet::mysleep(1);
}

void task2() {
  cout << "task2...runing\n";
  ccnet::mysleep(1);
}

int main()
{
  //test(1);
  //test(1);
  //test(5);
  //test(10);
  //test(50);
  // 设置时区
  ccnet::TimeZone tz("/usr/share/zoneinfo/Asia/Shanghai");
  ccnet::Logger::setTimeZone(tz);
  ccnet::ThreadPool pool("threadpool_test");
  LOG_INFO << pool.name();
  pool.setMaxQueueSize(5);
  pool.start(9);
  pool.run(task1);
  pool.run(task2);
  pool.stop();



}
