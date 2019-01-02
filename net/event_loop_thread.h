// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is a public header file, it must only include public header files.

#ifndef CCNET_NET_EVENTLOOPTHREAD_H
#define CCNET_NET_EVENTLOOPTHREAD_H

#include "../base/condition.h"
#include "../base/mutex.h"
#include "../base/thread.h"

namespace ccnet
{
///
/// 该类封装了IO线程
/// 任何一个线程，只要创建并运行了EventLoop，都称为IO线程
///
namespace net
{

class EventLoop;

class EventLoopThread : noncopyable
{
 public:
  typedef std::function<void(EventLoop*)> ThreadInitCallback;
  
  // 默认是空的回调函数
  EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(),
                  const string& name = string());
  ~EventLoopThread();
  EventLoop* startLoop(); // 启动线程，该线程称为了IO线程


 private:
  void threadFunc(); // 线程函数

  EventLoop* loop_; // loop_指向一个EventLoop对象
  bool exiting_;
  Thread thread_;
  MutexLock mutex_;
  Condition cond_;
  ThreadInitCallback callback_; // 在EventLoop::loop事件循环之前调用，做一些初始化操作
};

}
}

#endif  // CCNET_NET_EVENTLOOPTHREAD_H

