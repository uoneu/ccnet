#include "../event_loop.h"
#include "../poller.h"
#include "../event_loop_thread.h"
#include "../../base/current_thread.h"
#include "../../base/timestamp.h"
#include <iostream>
#include <functional>
#include <stdio.h>

using namespace ccnet;
using namespace ccnet::net;

/*
void my_thread() {
	printf("pid = %d, tid = %d\n", getpid(), current_thread::tid());

}

int main(void) {
    printf("pid = %d, tid = %d\n", getpid(), current_thread::tid());
    
    EventLoopThread loopthread;
    EventLoop *loop = loopthread.startLoop();
    loop->runInLoop(my_thread); // 异步调用my_thread， 即将my_thread添加到loop对象所在IO线程，让该IO线程执行
    sleep(1);
    loop->runAfter(2, my_thread); // 异步调用, 其内部也调用了runInLoop

    sleep(5);


}*/


void print(EventLoop* p = NULL)
{
  printf("print: pid = %d, tid = %d, loop = %p\n",
         getpid(), current_thread::tid(), p);
}

void quit(EventLoop* p)
{
  print(p);
  p->quit();
  printf("quit!\n");
}

int main()
{
  print();

  {
  EventLoopThread thr1;  // never start
  }

  {
  // dtor calls quit()
  EventLoopThread thr2;
  EventLoop* loop = thr2.startLoop();
  loop->runInLoop(std::bind(print, loop));
  current_thread::sleepUsec(500 * 1000);
  } // 离开此作用域，首先调用thr2析构函数，然后调用thread析构，

  {
  // quit() before dtor
  EventLoopThread thr3;
  EventLoop* loop = thr3.startLoop();
  loop->runInLoop(std::bind(quit, loop));
  current_thread::sleepUsec(500 * 1000);
  }
}

